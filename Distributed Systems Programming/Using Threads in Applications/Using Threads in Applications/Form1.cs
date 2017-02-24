using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;

namespace Using_Threads_in_Applications
{
    public partial class Form1 : Form
    {
        public List<int> primeNumbers;

        public Form1()
        {
            InitializeComponent();
            primeNumbers = new List<int>();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            ParameterizedThreadStart ts = new ParameterizedThreadStart(FindPrimeNumbers);
            ts.BeginInvoke(10000, new AsyncCallback(FindPrimesFinished), null);
        }

        private void FindPrimesFinished(IAsyncResult iar)
        {
            textBox1.Invoke(new MethodInvoker(UpdateTextBox));
        }

        private void UpdateTextBox()
        {
            int primeCount = primeNumbers.Count - 1;
            textBox1.Text = primeNumbers[primeCount].ToString();
        }

        private void FindPrimeNumbers(object param)
        {
            int numberOfPrimesToFind = (int)param;
            int primeCount = 0;
            int currentPossiblePrime = 1;

            while(primeCount < numberOfPrimesToFind)
            {
                int possibleFactor = 2;
                bool isPrime = true;

                currentPossiblePrime++;

                while((possibleFactor <= currentPossiblePrime / 2) && (isPrime == true))
                {
                    int possibleFactor2 = currentPossiblePrime / possibleFactor;

                    if (currentPossiblePrime == possibleFactor2 * possibleFactor)
                    {
                        isPrime = false;
                    }

                    possibleFactor++;
                }

                if(isPrime)
                {
                    primeCount++;
                    primeNumbers.Add(currentPossiblePrime);

                    textBox1.Invoke(new MethodInvoker(UpdateTextBox));
                }
            }
        }
    }
}