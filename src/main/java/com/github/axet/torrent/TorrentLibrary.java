package com.github.axet.torrent;

import java.io.File;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.concurrent.locks.ReentrantLock;

import com.github.axet.mavennatives.MavenNatives;
import com.github.axet.torrent.Torrent.State;
import com.rasterbar.libtorrent.TorrentLibrary.alert_type;
import com.rasterbar.libtorrent.TorrentLibrary.tags;
import com.rasterbar.libtorrent.endpoint;
import com.rasterbar.libtorrent.error_code;
import com.rasterbar.libtorrent.listen_succeeded_alert;
import com.rasterbar.libtorrent.torrent_checked_alert;
import com.rasterbar.libtorrent.torrent_status;
import com.sun.jna.Memory;
import com.sun.jna.NativeLibrary;
import com.sun.jna.Pointer;

public class TorrentLibrary {

    com.rasterbar.libtorrent.TorrentLibrary lib;

    Pointer ses;

    // lock all threads requiring for session alerts
    Object lockThreadsWaitingForAlert = new Object();
    // list of alerts to check
    int[] alerts;
    // master notify for worker thread
    ReentrantLock lockMasterNotifyForAlert = new ReentrantLock();
    // the current matched alert
    Pointer alert = null;

    // pooling thread
    Thread t;
    boolean work = true;

    // global holder
    static TorrentLibrary inst;

    HashMap<Long, Torrent> map = new HashMap<Long, Torrent>();

    static final int UPLOAD_SPEED = 10 * 1024;
    static final int DOWNLOAD_SPEED = 300 * 1024;

    public static class TorrentState {
        public byte[] resumeData;
    }

    public TorrentLibrary() {
        MavenNatives.mavenNatives("torrent", new MavenNatives.LoadNatives() {
            @Override
            public void setPath(String libraryName, File targetFile) {
                NativeLibrary.addSearchPath(libraryName, targetFile.getParent());
                NativeLibrary.getInstance(libraryName);
            }
        });
    }

    /**
     * getGlobal object, or create you own instance by calling TorrentSession()
     * 
     * @return TorrentSession global object
     */
    public static synchronized TorrentLibrary getInstance() {
        if (inst != null)
            return inst;

        inst = new TorrentLibrary();
        inst.create();

        return inst;
    }

    public Torrent add(String source, File target) {
        if (source.startsWith("magnet")) {
            synchronized (this.map) {
                Pointer t;

                t = lib.session_add_torrent(ses, tags.TOR_MAGNETLINK, source, tags.TOR_SAVE_PATH,
                        target.getAbsolutePath(), tags.TAG_END);

                if (t == null)
                    throw new RuntimeException("Failed to add torrent");

                return map(new Torrent(this, t));
            }
        }

        synchronized (this.map) {
            Pointer t;

            t = lib.session_add_torrent(ses, tags.TOR_FILENAME, source, tags.TOR_SAVE_PATH, target.getAbsolutePath(),
                    tags.TAG_END);

            if (t == null)
                throw new RuntimeException("Failed to add torrent");

            return map(new Torrent(this, t));
        }
    }

    public Torrent add(TorrentState m, File target) {
        Pointer t;

        t = lib.session_add_torrent_data(ses, toPointer(m.resumeData), m.resumeData.length, target.getAbsolutePath());

        if (t == null)
            throw new RuntimeException("Failed to add torrent");

        return map(new Torrent(this, t));
    }

    Torrent map(Torrent t) {
        Long hash = getHash(t.t);
        this.map.put(hash, t);

        return t;
    }

    public TorrentState remove(Torrent t) {
        t.pause();

        byte[] buf = null;
        try {
            buf = t.saveState();
        } catch (RuntimeException e) {
        }

        synchronized (this.map) {
            lib.session_remove_torrent(ses, t.getPointer(), 0);

            this.map.remove(t.t);
        }

        TorrentState m = new TorrentState();
        m.resumeData = buf;

        if (m.resumeData == null)
            return null;
        else
            return m;
    }

    public void close() {
        synchronized (t) {
            work = false;
        }

        lib.session_close(ses);
    }

    /**
     * Convert status to string
     * 
     * @param st
     * @return readable string
     */
    public String status(torrent_status st) {
        String str;
        str = String.format("%.0f%% / down %.0f kB/s / up %.0f kB/s / peers %d / %s / pause: %d",
                (double) st.progress * 100., (double) st.download_payload_rate / 1000.,
                (double) st.upload_payload_rate / 1000., st.num_peers, State.values()[st.state], st.paused);
        return str;

    }

    /**
     * set upload limit
     * 
     * @param bps
     *            - bytes per second
     */
    public void setUploadLimit(int bps) {
        lib.session_set_settings(ses, tags.SET_UPLOAD_RATE_LIMIT, bps, tags.TAG_END);
    }

    /**
     * set download limit
     * 
     * @param bps
     *            - bytes per second
     */
    public void setDownloadLimit(int bps) {
        lib.session_set_settings(ses, tags.SET_DOWNLOAD_RATE_LIMIT, bps, tags.TAG_END);
    }

    public void check(torrent_status st) {
        if (strLen(st.error) > 0) {
            throw new RuntimeException(toString(st.error));
        }
    }

    // protected

    Pointer toPointer(byte[] buf) {
        Memory m = new Memory(buf.length);
        Pointer p = m;
        p.write(0, buf, 0, buf.length);
        return p;
    }

    static String toString(ByteBuffer buf) {
        String str = "";
        for (int i = 0; i < buf.capacity(); i++) {
            char cc = (char) buf.get(i);
            if (cc == 0)
                break;
            str += cc;
        }
        return str;
    }

    static String toString(byte[] buf) {
        String str = "";
        for (int i = 0; i < buf.length; i++) {
            char cc = (char) buf[i];
            if (cc == 0)
                break;
            str += cc;
        }
        return str;
    }

    static int strLen(byte[] buf) {
        int i;
        for (i = 0; i < buf.length; i++) {
            if (buf[i] == 0)
                break;
        }

        return i;
    }

    public void create() {
        lib = com.rasterbar.libtorrent.TorrentLibrary.INSTANCE;

        if (lib == null)
            throw new RuntimeException("unable to load library");

        ses = lib.session_create(tags.SES_LISTENPORT, new Integer(6881), tags.SES_LISTENPORT_END, new Integer(6889),
                tags.TAG_END);

        setUploadLimit(UPLOAD_SPEED);
        setDownloadLimit(DOWNLOAD_SPEED);

        t = new Thread(new Runnable() {
            @Override
            public void run() {
                while (work())
                    process();
            }
        }, "TorrentInstance Thread");
        t.start();
    }

    public boolean work() {
        synchronized (t) {
            return work;
        }
    }

    public void check(error_code ec) {
        if (ec.value > 0) {
            throw new RuntimeException("error_code: " + ec.value + ", " + ec.category.getString(0) + ", "
                    + ec.message.getString(0));
        }
    }

    void check(endpoint st) {
        check(st.error);
    }

    void process() {
        Pointer p;
        while ((p = lib.session_wait_alert(ses)) != null) {
            synchronized (lockMasterNotifyForAlert) {
                if (alerts != null) {
                    int tt = lib.alert_get_type(p);
                    for (int t : alerts) {
                        if (t == tt) {
                            alert = p;
                            alerts = null;

                            lockMasterNotifyForAlert.notify();
                            return;
                        }
                    }
                }
            }

            process_event_alerts(p);
        }
    }

    void processNoWait() {
        Pointer p;
        while ((p = lib.session_pop_alert(ses)) != null) {
            process_event_alerts(p);
        }
    }

    void process_event_alerts(Pointer p) {
        int t = lib.alert_get_type(p);

        // we have to process all event like alerts.
        //
        // function return values must not be here (like
        // at_torrent_paused_alert).
        //
        // all other alerts must be here (like at_listen_failed_alert).

        switch (t) {
        case alert_type.at_listen_succeeded_alert: {
            listen_succeeded_alert a = null;
            try {
                a = lib.cast_to_listen_succeeded_alert(p);
                check(a.endpoint);
            } finally {
                if (a != null)
                    lib.free_listen_succeeded_alert(a);
            }
            return;
        }
        case alert_type.at_torrent_checked_alert: {
            torrent_checked_alert a = null;
            try {
                a = lib.cast_to_torrent_checked_alert(p);
                Long hash = getHash(a.torrent_alert.handle);
                synchronized (this.map) {
                    Torrent h = this.map.get(hash);
                    h.checked();
                }
            } finally {
                if (a != null)
                    lib.free_torrent_checked_alert(a);
            }
            return;
        }
        }

        lib.session_free_alert(p);
    }

    /**
     * waitForAlert
     * 
     * @param i
     *            list of alert types we are waiting as a respond
     * @param r
     *            command issued after we enter the lock. we have to be inside
     *            lock while issuing the trigger command. otherwise we can lose
     *            alert
     * @return
     */
    Pointer waitForAlert(int[] i, Runnable r) {
        synchronized (lockThreadsWaitingForAlert) {
            alerts = i;
            synchronized (lockMasterNotifyForAlert) {
                r.run();
                while (true) {
                    try {
                        lockMasterNotifyForAlert.wait();
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                    return alert;
                }
            }
        }
    }

    Long getHash(Pointer p) {
        return lib.torrent_get_hash(p).longValue();
    }

    public static void main(String[] args) {
        System.out.println("start");

        TorrentLibrary lib = new TorrentLibrary();

        lib.create();

        String magnet = "magnet:?xt=urn:btih:1f75170b358e5307644272c6a1dc839ff8d873ff&dn=The+Girl+With+A+Dragon+Tattoo+2011+DVDSCR+XviD+AC3-FTW&tr=udp%3A%2F%2Ftracker.openbittorrent.com%3A80&tr=udp%3A%2F%2Ftracker.publicbt.com%3A80&tr=udp%3A%2F%2Ftracker.ccc.de%3A80";
        File target = new File("/Users/axet/Downloads/");

        Torrent t = lib.add(magnet, target);

        torrent_status st = new torrent_status();

        int count = 0;

        while (true) {
            System.out.println("count: " + count);

            st = t.status();

            System.out.println(lib.status(st));

            // if (count == 5)
            // t.recheck();

            if (count == 5)
                t.resume();

            if (count == 30) {
                TorrentState m = lib.remove(t);
                t = lib.add(m, target);
                System.out.println("t " + t);
                t.resume();
            }

            lib.check(st);

            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            }

            count++;
        }

        // lib.close();
    }

}
