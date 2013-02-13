package com.github.axet.torrent.rasterbar;
import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;
/**
 * <i>native declaration : c/libtorrent.h:451</i><br>
 * This file was autogenerated by <a href="http://jnaerator.googlecode.com/">JNAerator</a>,<br>
 * a tool written by <a href="http://ochafik.com/">Olivier Chafik</a> that <a href="http://code.google.com/p/jnaerator/wiki/CreditsAndLicense">uses a few opensource projects.</a>.<br>
 * For help, please visit <a href="http://nativelibs4java.googlecode.com/">NativeLibs4Java</a> , <a href="http://rococoa.dev.java.net/">Rococoa</a>, or <a href="http://jna.dev.java.net/">JNA</a>.
 */
public class block_downloading_alert extends Structure {
	/// C type : peer_alert
	public peer_alert peer_alert;
	public int block_index;
	public int piece_index;
	public block_downloading_alert() {
		super();
	}
    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[]{"peer_alert", "block_index", "piece_index"});
	}
	/// @param peer_alert C type : peer_alert
	public block_downloading_alert(peer_alert peer_alert, int block_index, int piece_index) {
		super();
		this.peer_alert = peer_alert;
		this.block_index = block_index;
		this.piece_index = piece_index;
	}
	public static class ByReference extends block_downloading_alert implements Structure.ByReference {
		
	};
	public static class ByValue extends block_downloading_alert implements Structure.ByValue {
		
	};
}
