import java.io.*;
import java.net.*;

public class RemoteHeadServer {
    public static void main(String[] args) {
        if(args.length!=1){
            System.err.println("Usage RemoteHeadServer porta");
            System.exit(1);
        }
        try{
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
            
            for(;;){
               
                Socket s = ss.accept(); //attesa richiesta di connessione
                BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));

                String filename=netIn.readLine(); //leggo filename dalla socket
                //controllo esistenza del file
                File f = new File(filename);
                if(f.exists()){
                   BufferedReader bfr = new BufferedReader(new FileReader(f));
                    String line;
                    int lineNumber=1;
                    //leggo dal file
                    while ((line=bfr.readLine())!= null && lineNumber<5) {
                        netOut.write(line); //mando sulla socket
                        netOut.newLine();
                        netOut.flush();
                        lineNumber++;
                    }

                }
                
                s.close();
            }
        }catch(IOException e){
            System.err.println(e.getMessage());
        }
       

    }
}
