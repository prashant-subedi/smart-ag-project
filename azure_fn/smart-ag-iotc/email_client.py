import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import datetime

#Email Account
email_sender_account = "hafiz.oyediran@gmail.com" #your email
email_sender_username = "hafiz.oyediran"  #your email username
email_sender_password = "ThinkTank21"#your email password
email_smtp_server = "smtp.gmail.com" #change if not gmail.
email_smtp_port = 587 #change if needed.
#Email Content
email_receivers = ["oyediran.hafiz@gmail.com"]
email_subject = "Smart IoT Notification"
email_body = """testing testing testing"""

def send_email(email_subject, email_receivers, email_body):
    #For loop, sending emails to all email recipients
    for recipient in email_receivers:
        print(f"Sending email to {recipient}")
        message = MIMEMultipart('alternative')
        message['From'] = email_sender_account
        message['To'] = recipient
        message['Subject'] = email_subject
        message.attach(MIMEText(email_body))
        text = message.as_string()
        server.sendmail(email_sender_account,recipient,text)
    #All emails sent, log out.
    server.quit()
    

#login to email server
server = smtplib.SMTP(email_smtp_server,email_smtp_port)
server.starttls()
server.login(email_sender_username, email_sender_password)

TimeNow = datetime.datetime.now()
send_email(email_subject, email_receivers, email_body)
print(f"End")