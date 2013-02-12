package com.github.axet.torrent;

import com.rasterbar.libtorrent.torrent_title;

public class TorrentInfo {

    public String name;
    public String comment;
    public String creator;
    public long mtime;

    public TorrentInfo(torrent_title t) {
        name = t.name != null ? t.name.getString(0) : null;
        comment = t.comment != null ? t.comment.getString(0) : null;
        creator = t.creator != null ? t.creator.getString(0) : null;
        mtime = t.mtime;
    }
}
