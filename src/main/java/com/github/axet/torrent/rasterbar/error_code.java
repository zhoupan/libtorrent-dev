package com.github.axet.torrent.rasterbar;
import java.util.Arrays;
import java.util.List;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
/**
 * <i>native declaration : c/libtorrent.h:285</i><br>
 * This file was autogenerated by <a href="http://jnaerator.googlecode.com/">JNAerator</a>,<br>
 * a tool written by <a href="http://ochafik.com/">Olivier Chafik</a> that <a href="http://code.google.com/p/jnaerator/wiki/CreditsAndLicense">uses a few opensource projects.</a>.<br>
 * For help, please visit <a href="http://nativelibs4java.googlecode.com/">NativeLibs4Java</a> , <a href="http://rococoa.dev.java.net/">Rococoa</a>, or <a href="http://jna.dev.java.net/">JNA</a>.
 */
public class error_code extends Structure {
	public int value;
	/// C type : char*
	public Pointer category;
	/// C type : char*
	public Pointer message;
	public error_code() {
		super();
	}
    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[]{"value", "category", "message"});
	}
	/**
	 * @param category C type : char*<br>
	 * @param message C type : char*
	 */
	public error_code(int value, Pointer category, Pointer message) {
		super();
		this.value = value;
		this.category = category;
		this.message = message;
	}
	public static class ByReference extends error_code implements Structure.ByReference {
		
	};
	public static class ByValue extends error_code implements Structure.ByValue {
		
	};
}
