using System;
using System.IO;
using System.Net.Sockets;

namespace Client
{
    class Client
    {
        static byte[] rawData;

        static void Main(string[] args)
        {
            bool whileContinue = true;

            while (whileContinue)
            {
                string yn = "yn";

                while (yn != "Y" && yn != "N")
                {
                    Console.Write("Enter <Y> to view a webpage or enter <N> to view an encryption: ");
                    yn = Console.ReadLine().ToUpper();

                    if (yn == "N")
                    {
                        Encryption();
                    }
                    else
                    {
                        if (yn == "Y")
                        {
                            Webpage();
                        }
                    }
                }

                yn = "yn";

                while (yn != "Y" && yn != "N")
                {
                    Console.Write("Enter <Y> to continue or enter <N> to exit: ");
                    yn = Console.ReadLine().ToUpper();

                    if (yn == "N")
                    {
                        whileContinue = false;
                    }
                }
            }

            Console.Write("Press <ENTER> to quit the client...");
            Console.ReadLine();
        }

        public static void Webpage()
        {
            Console.Write("Please enter the URL to display (or just press <ENTER> to display http://www2.hull.ac.uk/science/computer_science.aspx): ");
            string host = Console.ReadLine();

            if (host == "")
            {
                host = "http://www2.hull.ac.uk/science/computer_science.aspx";
            }

            string subpath = "/";
            string[] hostParts = host.Split(new[] { "//" }, StringSplitOptions.None);

            if (hostParts.Length > 1)
            {
                host = hostParts[1]; hostParts = host.Split('/');
            }

            if (hostParts.Length > 1)
            {
                for (int i = 0; i < hostParts.Length; i++)
                {
                    if (i == 0)
                    {
                        host = hostParts[i];
                    }
                    else
                    {
                        if (i < hostParts.Length - 1)
                        {
                            subpath += hostParts[i] + "/";
                        }
                        else
                        {
                            subpath += hostParts[i];
                        }
                    }
                }
            }

            Console.WriteLine("Host: " + host);
            Console.WriteLine("GET path: " + subpath);

            int port = 80; // Default port for HTTP

            TcpClient client = new TcpClient(host, port); // Construct IO streams on the TcpClient's network stream

            NetworkStream nStream = client.GetStream();
            StreamWriter sOut = new StreamWriter(nStream, System.Text.Encoding.ASCII);
            StreamReader sIn = new StreamReader(nStream, System.Text.Encoding.ASCII);

            string rqst = "GET " + subpath;// Send HTTP request to the server

            sOut.WriteLine(rqst);
            sOut.Flush(); // Read the server's response and close the stream

            string msgIn = sIn.ReadToEnd();
            Console.Out.WriteLine(msgIn);

            sOut.Close();
            sIn.Close();
            nStream.Close();
        }

        public static void Encryption()
        {
            Console.Write("Please enter the destination IP address (or just press <ENTER> to listen at 127.0.0.1): ");
            string hostname = Console.ReadLine();

            if (hostname == "")
            {
                hostname = "127.0.0.1";
            }

            Console.Write("Please enter the port (or just press <ENTER> to listen at 5000): ");
            string portString = Console.ReadLine();
            int port = 5000;

            if (portString != "")
            {
                try
                {
                    port = int.Parse(portString);
                }
                catch
                {

                }
            }

            Console.Write("Please enter the message: ");
            string Message = Console.ReadLine();

            int EncryptionKey = 1;
            int MessageLength = Serialize(Message, EncryptionKey);

            TcpClient tcpClient = new TcpClient();
            tcpClient.Connect(hostname, port);

            NetworkStream nStream = tcpClient.GetStream();
            nStream.Write(rawData, 0, MessageLength);
            nStream.Flush();
            nStream.Close();

            tcpClient.Close();
        }

        public static int Serialize(string Message, int EncryptionKey)
        {
            int index = 0;
            byte[] ascii = System.Text.Encoding.ASCII.GetBytes(Message);

            int MessageLength = ascii.Length + 9;
            rawData = new byte[MessageLength];
            rawData[index++] = 0;
            rawData[index++] = 0;
            rawData[index++] = 0;
            rawData[index++] = 1;

            int remainingLength = ascii.Length + 3;
            rawData[index++] = (byte)((remainingLength & 0xff00) >> 8);
            rawData[index++] = (byte)(remainingLength & 0xff);
            rawData[index++] = (byte)EncryptionKey;

            int bodyLength = ascii.Length; rawData[index++] = (byte)((bodyLength & 0xff00) >> 8);
            rawData[index++] = (byte)(bodyLength & 0xff); Array.Copy(ascii, 0, rawData, index, ascii.Length);

            return MessageLength;
        }
    }
}