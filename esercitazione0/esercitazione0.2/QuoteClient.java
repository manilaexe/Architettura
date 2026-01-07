import java.io.*;
import java.net.*;
//interfaccia: java QuoteClient nodoServer portaServer
//                              args[0]    args[1]
public class QuoteClient {
    public static void main(String[] args) {
        if(args.length!=2){
            System.err.println("Usage: java QuoteClient hostname portaServer");
            System.exit(1);
        }
        try {
            DatagramSocket ds = new DatagramSocket(); //creazione socket

            byte[] reqBuff=new String("QUOTE").getBytes("UTF-8"); //stringa quote convertita a UTF-8
                                                                                        // QUOTE e` il "comando" che serve per fare richiesta
            DatagramPacket reqPacket = new DatagramPacket(reqBuff, reqBuff.length, InetAddress.getByName(args[0]), Integer.parseInt(args[1])); //richiesta da mandare al server per avere la quote
            //invio richiesta
            ds.send(reqPacket);
            //preparazione alla ricezione della risposta 
            byte[] respBuff = new byte[2048];
            DatagramPacket respPacket = new DatagramPacket(respBuff,respBuff.length);
            ds.receive(respPacket);
            //stampa a video la risposta
            String quote = new String(respPacket.getData(), 0, respPacket.getLength(), "UTF-8"); //conversione in stringa
            System.out.println(quote);
            ds.close();
        }catch (IOException e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}
