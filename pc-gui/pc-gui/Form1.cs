using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Text.Json;

namespace pc_gui
{
    public partial class Form1 : Form
    {
        private const string SERVER_ADDRESS = "raspberrypi.local";
        private const int SERVER_PORT = 8443;
        private const string CERTIFICATE_RELATIVE_PATH = "certs/rpi_cert.pem";

        private TcpClient client = null;
        private SslStream sslStream = null;
        private Thread updateTaskHandler = null;
        private System.Windows.Forms.Timer swTimer = null;

        private bool isConnected = false;
        private string gasSensorValue = "0";
        private string tmpSensorValue = "0.0";
        private string fanState = "Off";

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            StoreSSLCertificate();
            updateTaskHandler = new Thread(new ThreadStart(UpdateTask));

            swTimer = new System.Windows.Forms.Timer();
            swTimer.Interval = 1000; // each second
            swTimer.Tick += new EventHandler(UIUpdateTask);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (client != null)
            {
                client.Close();
            }
            if (sslStream != null)
            {
                sslStream.Close();
            }
            if (updateTaskHandler != null)
            {
                updateTaskHandler.Join();
            }
            if(swTimer.Enabled)
            {
                swTimer.Stop();
            }
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            client = new TcpClient(SERVER_ADDRESS, SERVER_PORT);
            sslStream = new SslStream(client.GetStream(), false, new RemoteCertificateValidationCallback(ValidateServerCertificate), null);
            try
            {
                sslStream.AuthenticateAsClient(SERVER_ADDRESS);
                isConnected = true;
                btnConnect.Enabled = false;
                swTimer.Start();
                updateTaskHandler.Start();
            }
            catch
            {
                
            }
        }

        private void UpdateTask()
        {
            if (sslStream != null)
            {
                while (true)
                {
                    byte[] buffer = new byte[1024];
                    sslStream.Read(buffer, 0, buffer.Length);
                    string decodedBuffer = Encoding.UTF8.GetString(buffer).TrimEnd('\0');

                    Dictionary<String, Object> jsonOutput = JsonSerializer.Deserialize<Dictionary<String, Object>>(decodedBuffer);
                    foreach(KeyValuePair<String, Object> entry in jsonOutput)
                    {
                        if("gas_sensor_value" == entry.Key)
                        {
                            gasSensorValue = entry.Value.ToString();
                        }
                        else if ("tmp_sensor_value" == entry.Key)
                        {
                            tmpSensorValue = entry.Value.ToString();
                        }
                        else if("fan_state" == entry.Key)
                        {
                            fanState = ("True" == entry.Value.ToString()) ? "On" : "Off";
                        }
                    }
                }
            }
        }

        private void UIUpdateTask(object sender, EventArgs e)
        {
            tbConnection.Text = isConnected ? "Yes" : "No";
            tbGasSensorValue.Text = gasSensorValue;
            tbTmpSensorValue.Text = tmpSensorValue;
            tbFanState.Text = fanState;
        }

        private bool ValidateServerCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            return sslPolicyErrors == SslPolicyErrors.None;
        }

        private void StoreSSLCertificate()
        {
            string currentPath = System.IO.Directory.GetCurrentDirectory();
            string certificatePath = String.Format("{0}/../../../../../{1}", currentPath, CERTIFICATE_RELATIVE_PATH);

            X509Store store = new X509Store(StoreName.Root, StoreLocation.CurrentUser);
            store.Open(OpenFlags.ReadWrite);
            store.Add(new X509Certificate2(certificatePath));
            store.Close();
        }
    }
}
