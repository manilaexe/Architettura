import java.io.*;
import java.net.*;

public class RemoteSquareServer {
    
    public static void main(String[] args) {
        if (args.length!=1){
            System.err.println("Usage RemoteSquareServer porta");
            System.exit(1);
        }
        try {
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
            int sqr, n;
            String line;

            for(;;){
                Socket s = ss.accept();
                BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));

                while((line=netIn.readLine())!=null){
                    if(line.equals("fine")){
                        break;
                    }

                    n=Integer.parseInt(line);
                    sqr=n*n;

                    netOut.write(Integer.toString(sqr));
                    netOut.newLine();
                    netOut.flush();     
                     
                }
                s.close();
            }
           
            
        } catch (IOException e) {
            System.err.println(e.getMessage());
            System.exit(1);
        }
    }
}
