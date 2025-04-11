
/**
  * Servidor da hora do dia escutando na porta 6013.
 *
 * Figure 3.21
 *
 * @author Silberschatz, Gagne, and Galvin. 
 * Fundamentos de Sistemas Operacionais � Nona Edi��o
 * Copyright John Wiley & Sons - 2013.
 */
 
import java.net.*;
import java.io.*;

public class DateServer
{
	public static void main(String[] args)  {
		try {
			ServerSocket sock = new ServerSocket(6013);

			// agora procura conex�es
			while (true) {
				Socket client = sock.accept();
				// temos uma conex�o
				
				PrintWriter pout = new PrintWriter(client.getOutputStream(), true);
				// grava a data no socket
				pout.println(new java.util.Date().toString());

				// fecha o socket e volta a procurer conex�es
				client.close();
			}
		}
		catch (IOException ioe) {
				System.err.println(ioe);
		}
	}
}
