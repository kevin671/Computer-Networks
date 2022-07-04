package minesweeper.server;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.NoSuchElementException;
import java.util.Optional;
import java.util.Queue;
import java.lang.Thread;

public class MinesweeperServerMulti {
	private static final int DEFAULT_PORT = 4444;
	private static final int MAXIMUM_PORT = 65535;
	private static final int DEFAULT_SIZE = 10;
	
	private final ServerSocket serverSocket;
	private final boolean debug;
	
	public MinesweeperServerMulti(int port, boolean debug) throws IOException {
		serverSocket = new ServerSocket(port);
		this.debug = debug;
	}
	
	public void serve() throws IOException {
		while(true) {
			Socket socket = serverSocket.accept();
			
			Thread handler = new Thread(new Runnable() {
				public void run() {
					try {
						try {
							handleConnection(socket);
						}finally {
							socket.close();
						}
					} catch(IOException ioe) {
						ioe.printStackTrace();
					}
				}
			});
			
			handler.start();
		}
	}
	
	public void handleConnection(Socket socket) throws IOException{
		BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
		
		try {
			for (String line = in.readLine(); line != null; line = in.readLine()) {
				String output = handleRequest(line);
				if (output != null) {
					out.println(output);
				}
			}
		}finally {
			out.close();
			in.close();
		}
		
	}
	
	private String handleRequest(String input) {
        String regex = "(look)|(help)|(bye)|"
                     + "(dig -?\\d+ -?\\d+)|(flag -?\\d+ -?\\d+)|(deflag -?\\d+ -?\\d+)";
        if ( ! input.matches(regex)) {
        }
        String[] tokens = input.split(" ");
        if (tokens[0].equals("look")) {
        } else if (tokens[0].equals("help")) {
        } else if (tokens[0].equals("bye")) {
        } else {
            int x = Integer.parseInt(tokens[1]);
            int y = Integer.parseInt(tokens[2]);
            if (tokens[0].equals("dig")) {
            } else if (tokens[0].equals("flag")) {
            } else if (tokens[0].equals("deflag")) {
            }
        }
        throw new UnsupportedOperationException();
    }
	
	public static void main(String[] args) {
		boolean debug = false;
        int port = DEFAULT_PORT;
        int sizeX = DEFAULT_SIZE;
        int sizeY = DEFAULT_SIZE;
        Optional<File> file = Optional.empty();
		
		Queue<String> arguments = new LinkedList<String>(Arrays.asList(args));
        try {
            while ( ! arguments.isEmpty()) {
                String flag = arguments.remove();
                try {
                    if (flag.equals("--debug")) {
                        debug = true;
                    } else if (flag.equals("--no-debug")) {
                        debug = false;
                    } else if (flag.equals("--port")) {
                        port = Integer.parseInt(arguments.remove());
                        if (port < 0 || port > MAXIMUM_PORT) {
                            throw new IllegalArgumentException("port " + port + " out of range");
                        }
                    } else if (flag.equals("--size")) {
                        String[] sizes = arguments.remove().split(",");
                        sizeX = Integer.parseInt(sizes[0]);
                        sizeY = Integer.parseInt(sizes[1]);
                        file = Optional.empty();
                    } else if (flag.equals("--file")) {
                        sizeX = -1;
                        sizeY = -1;
                        file = Optional.of(new File(arguments.remove()));
                        if ( ! file.get().isFile()) {
                            throw new IllegalArgumentException("file not found: \"" + file.get() + "\"");
                        }
                    } else {
                        throw new IllegalArgumentException("unknown option: \"" + flag + "\"");
                    }
                } catch (NoSuchElementException nsee) {
                    throw new IllegalArgumentException("missing argument for " + flag);
                } catch (NumberFormatException nfe) {
                    throw new IllegalArgumentException("unable to parse number for " + flag);
                }
            }
        } catch (IllegalArgumentException iae) {
            System.err.println(iae.getMessage());
            System.err.println("usage: MinesweeperServer [--debug | --no-debug] [--port PORT] [--size SIZE_X,SIZE_Y | --file FILE]");
            return;
        }

        try {
            runMinesweeperServer(debug, file, sizeX, sizeY, port);
        } catch (IOException ioe) {
            throw new RuntimeException(ioe);
        }
	}
	
	public static void runMinesweeperServer(boolean debug, Optional<File> file, int sizeX, int sizeY, int port) throws IOException {
		MinesweeperServerMulti server = new MinesweeperServerMulti(port, debug);
		server.serve();
	}
}
