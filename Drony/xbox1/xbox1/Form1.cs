using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using SharpDX.XInput;
using System.Net.Sockets;
using System.Net;
using System.IO.Ports;
namespace xbox1
{


    public partial class Form1 : Form
    {
        private NetworkStream stream;
        TcpClient client;
        XInputController xbox = new XInputController();
        int throttle = 1000;
        int pitch = 1333;
        int yaw = 1666;
        int roll = 2000;
        int aux1 = 1500;
        int aux2 = 1500;
        private bool connected = false;

        System.Windows.Forms.Timer t = new System.Windows.Forms.Timer();
        SerialPort _serialPort = new SerialPort("COM4", 115200);



        public Form1()
        {
            InitializeComponent();
            t.Tick += T_Tick;
            t.Interval = 200;
            t.Start();


        }

        private void SendDataToEsp(int throttle, int pitch, int roll, int yaw, int input1, int input2)
        {
            
            try
            {
                string command = "@" + throttle.ToString() + "#" + pitch.ToString() + "$" + roll.ToString() + "%" + yaw.ToString() + "^" + input1.ToString() + "&" + input2.ToString() + "e";
            byte[] message = Encoding.ASCII.GetBytes(command);
          //  stream.Write(message, 0, message.Length);
                _serialPort.WriteLine(command);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        private void ConnectAsClient()
        {
            
            try
            {
                client = new TcpClient();
                client.Connect(IPAddress.Parse("192.168.0.100"), 80);
                stream = client.GetStream();

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            connected = true;
        }



        private void chart1_Click(object sender, EventArgs e)
        {

        }

        private void T_Tick(object sender, EventArgs e)
        {
            if (xbox.connected)
            {
             //   Console.WriteLine((int)xbox.leftThumby);
                xbox.Update();
 
                yaw = (int)xbox.leftThumbx.Map(-100, 100, 1200, 1800);
                roll = (int)xbox.rightThumbx.Map(100, -100, 1200, 1800);
                pitch = (int)xbox.rightThumby.Map(-100, 100, 1200, 1800);
                throttle = (int)xbox.leftTrigger.Map(0, 255, 950, 2000);
                aux2 = (int)xbox.rightTrigger.Map(0, 255, 1000, 2000);
                aux1 = 1500;
                chart1.Series["values"].Points.Clear();
                chart1.Series["values"].Points.AddXY("Throttle", throttle);
                chart1.Series["values"].Points.AddXY("Pitch", pitch);
                chart1.Series["values"].Points.AddXY("Roll", roll);
                chart1.Series["values"].Points.AddXY("Yaw", yaw);
                chart1.Series["values"].Points.AddXY("Aux1", aux1);
                chart1.Series["values"].Points.AddXY("Aux2", aux2);

                button1.Text = "CONNECTED!";
                button1.ForeColor = Color.FromArgb(0, 255, 0);
                //           if (connected)
                //         {
                 //        SendDataToEsp(throttle, pitch, roll, yaw, aux1, aux2);
                         SendDataToEsp(pitch, roll, throttle, yaw, aux1, aux2);
                //     }

            }
            else
            {
                button1.Text = "DISCONNECTED!";
                button1.ForeColor = Color.FromArgb(255, 0, 0); ;
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {

        }

        private void Form1_Load(object sender, EventArgs e)
        {
            if(!_serialPort.IsOpen)
                {
                _serialPort.Open();
            }
        }

        private void splitContainer1_Panel2_Paint(object sender, PaintEventArgs e)
        {

        }

        private void label1_Click(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            Thread mThread = new Thread(new ThreadStart(ConnectAsClient));
            mThread.Start();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            connected = false;
            try
            {
                stream.Close();
                client.Close();

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }

    class XInputController
    {
        Controller controller;
        Gamepad gamepad;
        public bool connected = false;
        public int deadband = 2500;
        public float leftThumbx, leftThumby, rightThumbx, rightThumby;
        public float leftTrigger, rightTrigger;

        public XInputController()
        {
            controller = new Controller(UserIndex.One);
            connected = controller.IsConnected;
        }

        // Call this method to update all class values
        public void Update()
        {
            if (!connected)
                return;

            gamepad = controller.GetState().Gamepad;

            leftThumbx = (Math.Abs((float)gamepad.LeftThumbX) < deadband) ? 0 : (float)gamepad.LeftThumbX / short.MinValue * -100;
            leftThumby = (Math.Abs((float)gamepad.LeftThumbY) < deadband) ? 0 : (float)gamepad.LeftThumbY / short.MaxValue * 100;
            rightThumby = (Math.Abs((float)gamepad.RightThumbX) < deadband) ? 0 : (float)gamepad.RightThumbX / short.MaxValue * 100;
            rightThumbx = (Math.Abs((float)gamepad.RightThumbY) < deadband) ? 0 : (float)gamepad.RightThumbY / short.MaxValue * 100;

            leftTrigger = gamepad.LeftTrigger;
            rightTrigger = gamepad.RightTrigger;
        }
    }

    public static class ExtensionMethods
    {
        public static float Map(this float value, float fromSource, float toSource, float fromTarget, float toTarget)
        {
            return ((value - fromSource) / (toSource - fromSource) * (toTarget - fromTarget) + fromTarget);
        }
    }
}
