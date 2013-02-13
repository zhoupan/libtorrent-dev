package com.github.axet.torrent;

import java.util.ArrayList;
import java.util.List;

import com.rasterbar.libtorrent.file_entry;
import com.rasterbar.libtorrent.save_resume_data_alert;
import com.rasterbar.libtorrent.save_resume_data_failed_alert;
import com.rasterbar.libtorrent.torrent_status;
import com.rasterbar.libtorrent.torrent_title;
import com.rasterbar.libtorrent.TorrentLibrary.alert_type;
import com.sun.jna.Pointer;

public class Torrent {

    public static enum State {
        QUEUED, CHECKING, DOWNLOADING_METADATA, DOWNLOADING, FINISHED, SEEDING, ALLOCATING, CHECKING_RESUME_DATA
    };

    Pointer t;

    TorrentLibrary inst;
    com.rasterbar.libtorrent.TorrentLibrary lib;

    boolean pauseAfterChecked;

    public Torrent(TorrentLibrary inst, Pointer t) {
        this.t = t;
        this.inst = inst;

        lib = inst.lib;
    }

    Pointer getPointer() {
        return t;
    }

    public State state() {
        torrent_status st = torrentStatus();
        return State.values()[st.state];
    }

    torrent_status torrentStatus() {
        torrent_status st = new torrent_status();

        if (lib.torrent_get_status(t, st, st.size()) < 0)
            throw new RuntimeException("Unable to retrive status");

        return st;
    }

    public String status() {
        torrent_status st = torrentStatus();
        TorrentLibrary.check(st);

        return TorrentLibrary.status(st);
    }

    public long getTotal() {
        torrent_status st = torrentStatus();
        TorrentLibrary.check(st);
        return st.total_wanted;
    }

    public long getCount() {
        torrent_status st = torrentStatus();
        TorrentLibrary.check(st);
        return st.total_wanted_done;
    }

    public long getDownloadRate() {
        torrent_status st = torrentStatus();
        TorrentLibrary.check(st);
        return (long) st.download_payload_rate;
    }

    public TorrentInfo getInfo() {
        torrent_title tt = new torrent_title();
        lib.torrent_info(t, tt);
        TorrentInfo i = new TorrentInfo(tt);
        lib.torrent_info_free(t, tt);
        return i;
    }

    public void pause() {
        if (torrentStatus().paused == 1)
            return;

        Pointer p = inst.waitForAlert(new int[] { alert_type.at_torrent_paused_alert }, new Runnable() {
            @Override
            public void run() {
                if (lib.torrent_pause(t) < 0)
                    throw new RuntimeException("bad pause");
            }
        });
        lib.session_free_alert(p);
    }

    public void resume() {
        Pointer p = inst.waitForAlert(new int[] { alert_type.at_torrent_resumed_alert }, new Runnable() {
            @Override
            public void run() {
                if (lib.torrent_resume(t) < 0)
                    throw new RuntimeException("bad resume");
            }
        });
        lib.session_free_alert(p);
    }

    public byte[] saveState() {
        Pointer p = inst.waitForAlert(new int[] { alert_type.at_save_resume_data_alert,
                alert_type.at_save_resume_data_failed_alert }, new Runnable() {
            @Override
            public void run() {
                if (lib.torrent_save_state(t) < 0)
                    throw new RuntimeException("unable to save");
            }
        });

        int tt = lib.alert_get_type(p);

        switch (tt) {
        case alert_type.at_save_resume_data_alert: {
            save_resume_data_alert a = lib.cast_to_save_resume_data_alert(p);
            byte[] buf = a.resume_data.getByteArray(0, a.resume_data_size);
            lib.free_save_resume_data_alert(a);

            return buf;
        }
        case alert_type.at_save_resume_data_failed_alert: {
            save_resume_data_failed_alert a = lib.cast_to_save_resume_data_failed_alert(p);
            inst.check(a.error);
            lib.free_save_resume_data_failed_alert(a);
        }
        default:
            throw new RuntimeException("bad save state exception");
        }
    }

    public void recheck() {
        boolean p = torrentStatus().paused == 1;

        pauseAfterChecked = p;

        lib.torrent_recheck(t);

        if (p) {
            if (lib.torrent_resume(t) < 0)
                throw new RuntimeException("bad resume");
        }
    }

    public List<TorrentFileEntry> files() {
        List<TorrentFileEntry> list = new ArrayList<TorrentFileEntry>();

        int count = lib.torrent_files_count(t);
        for (int i = 0; i < count; i++) {
            file_entry f = new file_entry();
            lib.torrent_files_get(t, i, f);
            list.add(new TorrentFileEntry(i, f));
            lib.torrent_files_free(t, f);
        }

        return list;
    }

    public void rename(TorrentFileEntry fe, String path) {
        lib.torrent_files_rename(t, fe.index, path);
    }

    public boolean checking() {
        State s = state();
        return s == State.CHECKING || s == State.QUEUED || s == State.CHECKING_RESUME_DATA;
    }

    public void checked() {
        if (state() == State.CHECKING)
            return;

        if (pauseAfterChecked) {
            if (lib.torrent_pause(t) < 0)
                throw new RuntimeException("bad pause");

            pauseAfterChecked = false;
        }
    }
}
