//interfaccia java RemoteSquareClient hostname porta
//                                    args[0] args[1]

import java.io.*;
import java.net.*;
import java.util.*;

public class RemoteSquareClient {
    public static void main(String[] args) {
        //controllo dei parametri
        if(args.length!=2){
            System.err.println("Uso: java RemoteSquareClient hostname porta");
            System.exit(1);
        }
        String n;
        String risp;
        Scanner scn = new Scanner(System.in);

        try {
            Socket s = new Socket(args[0], Integer.parseInt(args[1])); //Socket(host,porta)
            //estremi di lettura
            BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));

            while(true){
                System.out.println("Inserire un numero ('fine' per terminare): ");
                n=scn.nextLine();
                if(n.equals("fine")){
                    break;
                }

                netOut.write(n);
                netOut.newLine();
                netOut.flush();

                risp=netIn.readLine();

                System.out.println("Risultato: " + risp);    
            }

            s.close();
            scn.close();
            
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(1);
        }
    }
}
