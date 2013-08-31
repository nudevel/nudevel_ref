package net.ipv4;

import java.net.InetAddress;
import java.net.UnknownHostException;


public class NewAddr extends Thread {
	private NewAddrListener privateListener;
	private boolean privateAlive;
	private int privatePollingInterval; /* msec */
	
	public void start(int pollingInterval /* msec */, NewAddrListener listener) {
		privateAlive = true;
		privateListener = listener;
		privatePollingInterval = pollingInterval;
		super.start();
	}

	
	
	public void run() {
		try{
			InetAddress lastAddr = InetAddress.getByAddress( new byte[4] );
			
			while(privateAlive == true){
				InetAddress newAddr = InetAddress.getLocalHost();
				if( lastAddr.equals(newAddr) == false ){
					privateListener.Notify(newAddr);
					lastAddr = newAddr;
				}
				sleep(privatePollingInterval);
			}
		}
		catch(UnknownHostException e){
			e.printStackTrace();
			privateListener.Notify(null);
		} catch (InterruptedException e) {
			e.printStackTrace();
			privateListener.Notify(null);
		}
	}
	
	public void close() {
		privateAlive = false;
	}
	
}
