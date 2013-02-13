package com.github.axet.torrent;

import com.github.axet.torrent.rasterbar.file_entry;

public class TorrentFileEntry {
    public final int index;

    public String path;
    public long size;
    public long mtime;
    public boolean pad_file;
    public boolean hidden_attribute;
    public boolean executable_attribute;
    public boolean symlink_attribute;
    public String symlink_path;

    public TorrentFileEntry(int i, file_entry f) {
        index = i;

        path = f.path.getString(0);
        size = f.size;
        mtime = f.mtime;
        pad_file = f.pad_file == 1;
        hidden_attribute = f.hidden_attribute == 1;
        executable_attribute = f.executable_attribute == 1;
        symlink_attribute = f.symlink_attribute == 1;
        symlink_path = f.symlink_path.getString(0);
        if (symlink_path.isEmpty())
            symlink_path = null;
    }
}
