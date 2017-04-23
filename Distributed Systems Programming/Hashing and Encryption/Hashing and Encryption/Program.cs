using System;
using System.Linq;
using System.Security.Cryptography;

namespace Hashing_and_Encryption
{
    class Program
    {
        static void Main(string[] args)
        {
            Message();
        }

        private static void Message()
        {
            string message = "Alex";

            byte[] unicodeByteMessage = System.Text.Encoding.Unicode.GetBytes(message);

            byte[] sha256ByteMessage;
            SHA256 sha256Provider = new SHA256CryptoServiceProvider();
            sha256ByteMessage = sha256Provider.ComputeHash(unicodeByteMessage);

            Console.WriteLine(ByteArrayToHexString(sha256ByteMessage));
            Console.WriteLine();

            try
            {
                string D = "0c5c750943dcfa8545fc926d07d2a2148a146a8d0359e22426e7dd00422820dcfd0ffe1454bbd83e821dbfde8916f3f92cef7c0ac37b8fdde431dd62d4cefc91b6633e342711db86a03e53e025ad1b2b5fd98ca7533ca76b2d1fecc85e5cc6921172804737eed46e38cd08f9aa845136e4895899bc9581b8ab80a37a217871cd";
                string DP = "3d8a50d650ae55aaedba2276ffe8976e256b601b59d4a13a77562345dee9ba250a5f210814ece48864ea9f16c9ab0c9834f5967ee59bafd5bf057e2e9761b96d";
                string DQ = "96ccd1e1b2bad0a2c030af827cf721d0804c466feb3109f1308084f4ac9406eccbef08d8401a806bd5896509edd0e4d4aee873c2d4697f93af6661ffe314df0d";
                string Exponent = "010001";
                string InverseQ = "4a3376fe05461bb23d380ba94f9da7127fc2ff83df4e81759781ec7f8d69b2ce396bcdae1e62719e68338a85294b8847f628a22c96afb299c36f10027b610685";
                string Modulus = "8ac707555bb033abe8d68dc38923cd87fa40ccf34cc337acb8fa7dffbe6929e8726a4f2bde3d652a66aca772d4b718476f4f38d301a8a84a07669436ea0afaed686af06f14df35119071b4b27cfafe55b923652a32a83ec1dc63426f348062c4bdfa7e3d4ba81b344b1e067c83f6fa1551f9a7b75cc51b9577969228e40bb8df";
                string P = "b724818dad3d4a2842d4d3f496beaf3d69685690f5b3440a6c1df9c4ecdc6ad71c341e4ee28de2a0abb58e23f3d6b050bd810d30269fc985170808f9abcb4793";
                string Q = "c1fc50b0fa425ace79728263e2cdc7deed58c1dbe1314b5410071a35e24963e1f744186ac5b884f7fec7b5b07dcbb46f3a73126c252f759f4452acef96dd4105";

                byte[] encryptedByteMessage;
                byte[] decryptedByteMessage;

                using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
                {
                    RSAParameters rsaParameters = new RSAParameters();

                    rsaParameters.D = StringToByteArray(D);
                    rsaParameters.DP = StringToByteArray(DP);
                    rsaParameters.DQ = StringToByteArray(DQ);
                    rsaParameters.Exponent = StringToByteArray(Exponent);
                    rsaParameters.InverseQ = StringToByteArray(InverseQ);
                    rsaParameters.Modulus = StringToByteArray(Modulus);
                    rsaParameters.P = StringToByteArray(P);
                    rsaParameters.Q = StringToByteArray(Q);

                    RSA.ImportParameters(rsaParameters);

                    encryptedByteMessage = RSAEncrypt(unicodeByteMessage, RSA.ExportParameters(false));
                    Console.Write("Encrypted message: ");
                    Console.WriteLine(ByteArrayToHexString(encryptedByteMessage));
                    Console.WriteLine();

                    decryptedByteMessage = RSADecrypt(encryptedByteMessage, RSA.ExportParameters(true));
                    Console.Write("Decrypted message: ");
                    Console.WriteLine(System.Text.Encoding.Unicode.GetString(decryptedByteMessage));
                    Console.WriteLine();
                }
            }
            catch
            {

            }

            Console.ReadLine();
        }

        public static byte[] StringToByteArray(string hex)
        {
            return Enumerable.Range(0, hex.Length).Where(x => x % 2 == 0).Select(x => Convert.ToByte(hex.Substring(x, 2), 16)).ToArray();
        }

        static public byte[] RSAEncrypt(byte[] DataToEncrypt, RSAParameters RSAKeyInfo)
        {
            try
            {
                byte[] encryptedData;

                using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
                {
                    RSA.ImportParameters(RSAKeyInfo);
                    encryptedData = RSA.Encrypt(DataToEncrypt, false);
                }

                return encryptedData;
            }
            catch (CryptographicException e)
            {
                Console.WriteLine(e.Message);

                return null;
            }
        }

        static public byte[] RSADecrypt(byte[] DataToDecrypt, RSAParameters RSAKeyInfo)
        {
            try
            {
                byte[] decryptedData;

                using (RSACryptoServiceProvider RSA = new RSACryptoServiceProvider())
                {
                    RSA.ImportParameters(RSAKeyInfo);
                    decryptedData = RSA.Decrypt(DataToDecrypt, false);
                }

                return decryptedData;
            }
            catch (CryptographicException e)
            {
                Console.WriteLine(e.ToString());

                return null;
            }
        }

        static string ByteArrayToHexString(byte[] byteArray)
        {
            string hexString = "";

            if (null != byteArray)
            {
                for (int i =0; i < byteArray.Length; i++)
                {
                    hexString += byteArray[i].ToString("x2");
                }
            }

            return hexString;
        }
    }
}
