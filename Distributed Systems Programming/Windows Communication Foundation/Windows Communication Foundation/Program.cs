using System;
using System.ServiceModel;
using WcfTranslationLibrary;

namespace Windows_Communication_Foundation
{
    class Program
    {
        static void Main(string[] args)
        {
            ServiceHost myHost = new ServiceHost(typeof(TranslationService));
            myHost.Open();

            Console.WriteLine("Translator running...");
            Console.ReadLine();

            myHost.Close();
        }
    }
}
