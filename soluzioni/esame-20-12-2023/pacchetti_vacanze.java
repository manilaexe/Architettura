import java.io.*;
import java.net.*;

public class ClientTDConnreuse
{
    public static void main(String[] args)
    {
        if (args.length != 2)
        {
            System.err.println("Usage: java ClientTDConnreuse <hostname> <port>");
            System.exit(1);
        }

        try
        {
            // Create a socket to connect to the server
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));
            BufferedReader userIn = new BufferedReader(new InputStreamReader(System.in));

            while(true) {

                System.out.print("Inserisci il mese: ");
                var mese = userIn.readLine();

                if (mese.equals("fine")) {
                    break;
                }

                System.out.print("Inserisci la tipologia: ");
                var tipologia = userIn.readLine();

                System.out.print("Inserisci la località: ");
                var località = userIn.readLine();

                netOut.write(mese + "\n");
                netOut.write(tipologia + "\n");
                netOut.write(località + "\n");
                netOut.flush();

                String risposta;
                while ((risposta = netIn.readLine()) != null) {

                    if (risposta.equals("---END RESPONSE---")) {
                        break;
                    }

                    System.out.println(risposta);
                }
            }

            // Close the socket
            socket.close();
        }
        catch (Exception e)
        {
            e.printStackTrace(System.err);
            System.err.println(e.getMessage());
            System.exit(1);
        }
    }
}
            
