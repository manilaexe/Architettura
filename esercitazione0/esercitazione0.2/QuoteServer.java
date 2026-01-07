import java.io.*;
import java.net.*;
public class QuoteServer {
    static final String[] quotations={
        "Chi dorme non piglia pesci",
        "Il tempo e` il miglior maestro, ma uccide tutti i suoi allievi",
        "Non tutto il male viene per nuocere",
        "La fortuna aiuta gli audaci",
        "Meglio tardi che mai",
        "Il dubbio e` l'inizio della conoscenza",
        "La conoscenza parla, la saggezza ascolta",
        "La vita e` quello che succede mentre fai altri piani",
        "Sbagliando si impara",
        "Non esiste vento favorevole per il marinaio che non sa dove andare"
    };

    public static void main(String[] args) {
        if(args.length!=1){
            System.err.println("Usage; java QuoteServer porta");
            System.exit(1);
        }

        
        try{
            int index=0; //indice quella quote corrente
            //creazione della socket
            DatagramSocket ds = new DatagramSocket(Integer.parseInt(args[0]));
            //ciclo di richiesta e risposta
            while(true){
                //creazione buffer e datagram packet
                byte[] reqBuff = new byte[2048]; //creo un buffer (array di byte)
                DatagramPacket reqPacket = new DatagramPacket(reqBuff,reqBuff.length);
                //lettura del messaggio di richiesta
                ds.receive(reqPacket);
                //estrazione dela stringa di richiesta
                String request=new String(reqPacket.getData(), 0, reqPacket.getLength(), "UTF-8"); //specifico offset e length
                if(request.equals("QUOTE")){
                    //ottengo prossima quote
                    String quote=quotations[index%quotations.length];
                    //concersione della quote da stringa a sequenza di byte
                    byte[] respBuff=quote.getBytes("UTF-8");
                    //preparazione DatagramPacket di risposta
                    DatagramPacket respPacket = new DatagramPacket(respBuff, respBuff.length, reqPacket.getAddress(), reqPacket.getPort());
                    //trasmissione della risposta al client
                    ds.send(respPacket);
                    index++;
                }
            }
            
        }catch(IOException e){
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}
