import java.io.*;
import java.net.*;
import java.util.*;

public class Client{
	public static void main(String[] args) {
		if(args.length!=2){
			System.err.println("Usage: esamina_spese_principali server porta ");
			System.exit(1);
		}
		String mese, n, cat, risp;
		Scanner scn = new Scanner(System.in);

		try{
			Socket s = new Socket(args[0], Integer.parseInt(args[1]));
			BufferedReader netIn = new BufferedReader(new InputStreamReader(s.getInputStream(), "UTF-8"));
			BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(s.getOutputStream(), "UTF-8"));
			
			while(true){
				System.out.println("Inserire il mese: ");
				mese=scn.nextLine();
				if(mese.equals("fine")){
					break;
				}
				System.out.println("Inserire la categoria: ");
				cat=scn.nextLine();
				System.out.println("Inserire il numero di righe che si desidera visualizzare: ");
				n=scn.nextLine();
				netOut.write(mese);
				netOut.newLine();
				netOut.write(cat);
				netOut.newLine();
				netOut.write(n);
				netOut.newLine();
				netOut.flush();

				while (true) { 
					risp=netIn.readLine();
					if(risp==null){
						break;
					}
					if(risp.equals("--- END REQUEST ---")){
						break;
					}
					System.out.println(risp);

				}
				

			}
			s.close();
			scn.close();
			
		}catch(IOException e){
			System.err.println(e.getMessage());
			System.exit(1);
		}
	}
}
