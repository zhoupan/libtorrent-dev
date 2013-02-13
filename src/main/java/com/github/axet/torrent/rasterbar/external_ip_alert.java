package com.github.axet.torrent.rasterbar;
import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;
/**
 * <i>native declaration : c/libtorrent.h:319</i><br>
 * This file was autogenerated by <a href="http://jnaerator.googlecode.com/">JNAerator</a>,<br>
 * a tool written by <a href="http://ochafik.com/">Olivier Chafik</a> that <a href="http://code.google.com/p/jnaerator/wiki/CreditsAndLicense">uses a few opensource projects.</a>.<br>
 * For help, please visit <a href="http://nativelibs4java.googlecode.com/">NativeLibs4Java</a> , <a href="http://rococoa.dev.java.net/">Rococoa</a>, or <a href="http://jna.dev.java.net/">JNA</a>.
 */
public class external_ip_alert extends Structure {
	/// C type : alert
	public alert alert;
	public int external_address;
	public external_ip_alert() {
		super();
	}
    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[]{"alert", "external_address"});
	}
	/// @param alert C type : alert
	public external_ip_alert(alert alert, int external_address) {
		super();
		this.alert = alert;
		this.external_address = external_address;
	}
	public static class ByReference extends external_ip_alert implements Structure.ByReference {
		
	};
	public static class ByValue extends external_ip_alert implements Structure.ByValue {
		
	};
}