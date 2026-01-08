import java.io.*;
import java.net.*;

public class RemoteDGramStrlenServer {
    public static void main(String[] args) {
        if(args.length!=1){
            System.err.println("Uso: java RemoteDGramStrlenServer porta");
            System.exit(1);
        }
        try {
            DatagramSocket ds = new DatagramSocket(Integer.parseInt(args[0]));
            int c; 
            String res;
            while(true){
                byte[] reqBuff = new byte[4096];
                DatagramPacket reqPkt = new DatagramPacket(reqBuff,reqBuff.length);
                //lettura del messagigo di richiesta
                ds.receive(reqPkt);
                //estrazione della stringa di richiesta
                String req = new String(reqPkt.getData(), 0, reqPkt.getLength(),"UTF-8");
                if(req.equals("fine")){
                    break;
                }
                c=req.length();
                res="I caratteri contenuti in '" + req +"' sono: " + c;
                byte[] resBuff=res.getBytes("UTF-8");
                DatagramPacket resPkt = new DatagramPacket(resBuff, resBuff.length, reqPkt.getAddress(), reqPkt.getPort());
                ds.send(resPkt); 

            }
        } catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);            
            // TODO: handle exception
        }
    }
}
