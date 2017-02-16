using System;
using System.Net;
using System.Net.Sockets;

namespace Server
{
    class Server
    {
        static void Main(string[] args)
        {
            Console.Write("Please enter the local IP address (or just press <ENTER> to listen at 127.0.0.1): ");
            string address = Console.ReadLine();

            if (address == "")
            {
                address = "127.0.0.1";
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
                    Console.WriteLine("Invalid entry, port set to 5000.");
                }
            }

            TcpListener tcpListener = new TcpListener(IPAddress.Parse(address), port);
            tcpListener.Start();

            Console.WriteLine("Listening at port " + port + "...");

            while (true)
            {
                TcpClient tcpClient = tcpListener.AcceptTcpClient();
                NetworkStream nStream = tcpClient.GetStream();

                byte[] header = new byte[6];

                nStream.Read(header, 0, 6);

                int messageType = header[3];
                int remainingLength = header[4] * 256 + header[5];

                Console.WriteLine("Message type: " + messageType);
                Console.WriteLine("Length: " + remainingLength);

                byte[] rawData = new byte[remainingLength];

                nStream.Read(rawData, 0, remainingLength);
                nStream.Flush();
                nStream.Close();

                tcpClient.Close();

                int offset = rawData[0];
                int dataLength = rawData[1] * 256 + rawData[2];

                byte[] byteMessage = new byte[remainingLength];
                Array.Copy(rawData, 3, byteMessage, 0, dataLength);
                string Message = System.Text.Encoding.ASCII.GetString(rawData, 3, dataLength);

                Console.WriteLine("Encryption key: " + offset);
                Console.WriteLine("Data length: " + dataLength);
                Console.WriteLine("Received message:");
                Console.WriteLine(Message);

                for (int i = 0; i < Message.Length; i++)
                {
                    byteMessage[i] = (byte)((int)byteMessage[i] + (int)offset);

                    if (byteMessage[i] > 126)
                    {
                        byteMessage[i] = (byte)((int)byteMessage[i] - 126 + 31);
                    }
                }

                string EncryptedMessage = System.Text.Encoding.ASCII.GetString(byteMessage, 0, dataLength);

                Console.WriteLine("Encrypted message:");
                Console.WriteLine(EncryptedMessage);
            }
        }
    }
}