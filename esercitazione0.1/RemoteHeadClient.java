//java RemoteHeadClient hostname porta   nomefile
//                      args[0]  args[1] args[2]
import java.io.*;
import java.net.*;
class RemoteHeadClient{
    public static void main (String args[]){
        if(args.length != 3){
            System.err.println("Usage: RemoteHeadClient hostname porta nomefile");
            System.exit(1);
        }
        try{
            Socket s = new Socket(args[0], Integer.parseInt(args[1]));
            String line;
            //estremi di lettura
            BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));

            //mando nome file
            netOut.write(args[2]); 
            netOut.newLine();
            netOut.flush();

            //lettura dati dal server
            int lineNumber=1;
            while((line=netIn.readLine())!=null && lineNumber < 5){
                System.out.println(line);
                lineNumber++;
            }
            s.close();  
        }catch(IOException e){
            System.err.println(e.getMessage());
            System.exit(1);
        }
        
    }
}
