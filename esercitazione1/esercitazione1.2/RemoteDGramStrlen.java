//interfaccia: java RemoteDGramStrlen nodoServer portaServer
//                                    args[0]    args[1]
import java.io.*;
import java.net.*;
import java.util.*;

public class RemoteDGramStrlen {
    public static void main(String[] args) {
        //controllo dei parametri
        if(args.length!=2){
            System.err.println("Uso: java RemoteDGramStrlen nodoServer portaServer");
            System.exit(1);
        }
        Scanner scanner = new Scanner(System.in);
        String req;
        String ns=args[0];
        int porta=Integer.parseInt(args[1]);


        try {
            DatagramSocket ds = new DatagramSocket();

            while(true){
                System.out.println("Inserire una frase per contarne i caratteri ('fine' per terminare): ");
                req=scanner.nextLine();
                if(req.equals("fine")){
                    break;
                }
                //invio la richiesta
                byte[] reqBuff = req.getBytes("UTF-8");
                DatagramPacket reqPacket = new DatagramPacket(reqBuff, reqBuff.length, InetAddress.getByName(ns), porta);
                ds.send(reqPacket);

                //ricezione riposta
                byte[] resBuff = new byte[4096];
                DatagramPacket resPkt = new DatagramPacket(resBuff, resBuff.length);
                ds.receive(resPkt);

                //stampa della risposta
                String ris = new String(resPkt.getData(), 0, resPkt.getLength());
                System.out.println(ris);
                
            }
            ds.close();
            scanner.close();
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(1);
        }


    }
}
