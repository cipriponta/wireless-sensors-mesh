
namespace pc_gui
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.tbGasSensorValue = new System.Windows.Forms.TextBox();
            this.tbTmpSensorValue = new System.Windows.Forms.TextBox();
            this.tbFanState = new System.Windows.Forms.TextBox();
            this.lbGasSensorValue = new System.Windows.Forms.Label();
            this.lbTmpSensorValue = new System.Windows.Forms.Label();
            this.lbFanState = new System.Windows.Forms.Label();
            this.lbConnection = new System.Windows.Forms.Label();
            this.tbConnection = new System.Windows.Forms.TextBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // tbGasSensorValue
            // 
            this.tbGasSensorValue.Location = new System.Drawing.Point(178, 62);
            this.tbGasSensorValue.Name = "tbGasSensorValue";
            this.tbGasSensorValue.ReadOnly = true;
            this.tbGasSensorValue.Size = new System.Drawing.Size(100, 23);
            this.tbGasSensorValue.TabIndex = 0;
            this.tbGasSensorValue.Text = "0";
            // 
            // tbTmpSensorValue
            // 
            this.tbTmpSensorValue.Location = new System.Drawing.Point(178, 103);
            this.tbTmpSensorValue.Name = "tbTmpSensorValue";
            this.tbTmpSensorValue.ReadOnly = true;
            this.tbTmpSensorValue.Size = new System.Drawing.Size(100, 23);
            this.tbTmpSensorValue.TabIndex = 1;
            this.tbTmpSensorValue.Text = "0.0";
            // 
            // tbFanState
            // 
            this.tbFanState.Location = new System.Drawing.Point(178, 142);
            this.tbFanState.Name = "tbFanState";
            this.tbFanState.ReadOnly = true;
            this.tbFanState.Size = new System.Drawing.Size(100, 23);
            this.tbFanState.TabIndex = 2;
            this.tbFanState.Text = "Off";
            // 
            // lbGasSensorValue
            // 
            this.lbGasSensorValue.AutoSize = true;
            this.lbGasSensorValue.Location = new System.Drawing.Point(75, 65);
            this.lbGasSensorValue.Name = "lbGasSensorValue";
            this.lbGasSensorValue.Size = new System.Drawing.Size(95, 15);
            this.lbGasSensorValue.TabIndex = 3;
            this.lbGasSensorValue.Text = "Gas Sensor Value";
            // 
            // lbTmpSensorValue
            // 
            this.lbTmpSensorValue.AutoSize = true;
            this.lbTmpSensorValue.Location = new System.Drawing.Point(28, 106);
            this.lbTmpSensorValue.Name = "lbTmpSensorValue";
            this.lbTmpSensorValue.Size = new System.Drawing.Size(142, 15);
            this.lbTmpSensorValue.TabIndex = 4;
            this.lbTmpSensorValue.Text = "Temperature Sensor Value";
            // 
            // lbFanState
            // 
            this.lbFanState.AutoSize = true;
            this.lbFanState.Location = new System.Drawing.Point(115, 145);
            this.lbFanState.Name = "lbFanState";
            this.lbFanState.Size = new System.Drawing.Size(55, 15);
            this.lbFanState.TabIndex = 5;
            this.lbFanState.Text = "Fan State";
            // 
            // lbConnection
            // 
            this.lbConnection.AutoSize = true;
            this.lbConnection.Location = new System.Drawing.Point(105, 21);
            this.lbConnection.Name = "lbConnection";
            this.lbConnection.Size = new System.Drawing.Size(65, 15);
            this.lbConnection.TabIndex = 6;
            this.lbConnection.Text = "Connected";
            // 
            // tbConnection
            // 
            this.tbConnection.Location = new System.Drawing.Point(178, 18);
            this.tbConnection.Name = "tbConnection";
            this.tbConnection.ReadOnly = true;
            this.tbConnection.Size = new System.Drawing.Size(100, 23);
            this.tbConnection.TabIndex = 7;
            this.tbConnection.Text = "No";
            // 
            // btnConnect
            // 
            this.btnConnect.Location = new System.Drawing.Point(88, 187);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(128, 23);
            this.btnConnect.TabIndex = 8;
            this.btnConnect.Text = "Connect";
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(311, 222);
            this.Controls.Add(this.btnConnect);
            this.Controls.Add(this.tbConnection);
            this.Controls.Add(this.lbConnection);
            this.Controls.Add(this.lbFanState);
            this.Controls.Add(this.lbTmpSensorValue);
            this.Controls.Add(this.lbGasSensorValue);
            this.Controls.Add(this.tbFanState);
            this.Controls.Add(this.tbTmpSensorValue);
            this.Controls.Add(this.tbGasSensorValue);
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tbGasSensorValue;
        private System.Windows.Forms.TextBox tbTmpSensorValue;
        private System.Windows.Forms.TextBox tbFanState;
        private System.Windows.Forms.Label lbGasSensorValue;
        private System.Windows.Forms.Label lbTmpSensorValue;
        private System.Windows.Forms.Label lbFanState;
        private System.Windows.Forms.Label lbConnection;
        private System.Windows.Forms.TextBox tbConnection;
        private System.Windows.Forms.Button btnConnect;
    }
}

