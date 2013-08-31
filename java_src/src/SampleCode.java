import java.io.IOException;
import java.net.InetAddress;

import net.ipv4.NewAddr;
import net.ipv4.NewAddrListener;

class NewAddrListenerImpl implements NewAddrListener {

	@Override
	public void Notify(InetAddress newAddr) {
		// TODO Auto-generated method stub
		if(newAddr==null){
			System.out.println("error");
		} else {
			System.out.println(newAddr);
		}
	}
	
}

public class SampleCode {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		System.out.println("Start.");
		
		try {
			NewAddr newAddr = new NewAddr();
			newAddr.start(2000 /* msec */, new NewAddrListenerImpl());
			
			while( System.in.read() != 'x' ) ;
			
			newAddr.close();
			newAddr.join();
		} catch (InterruptedException e) {
			System.out.println(e);
		} catch (IOException e) {
			System.out.println(e);
		}
		
		System.out.println("End.");
	}

}
