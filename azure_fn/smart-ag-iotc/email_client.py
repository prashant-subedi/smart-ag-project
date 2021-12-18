"""
Utility to send email
"""
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import os

EMAIL_SENDER_ACCOUNT = "hafiz.oyediran@gmail.com"
EMAIL_SENDER_USERNAME = "hafiz.oyediran@gmail.com"
EMAIL_SENDER_PASSWORD = os.getenv("EMAIL_PASSOWRD")
EMAIL_SMTP_SERVER = "smtp.gmail.com"
EMAIL_SMTP_PORT = 587

def setup_server():
<<<<<<< Updated upstream
    #Email account settings

    # start connection and login
=======
    """Setup/Authenticate SMTP server"""
>>>>>>> Stashed changes
    server = smtplib.SMTP(EMAIL_SMTP_SERVER, EMAIL_SMTP_PORT)
    server.starttls()
    server.login(EMAIL_SENDER_USERNAME, EMAIL_SENDER_PASSWORD)
    return server

def send_email(email_subject, email_receivers, email_body):
    """Send using SMTP protocol"""
    server = setup_server()
    #For loop, sending emails to all email recipients
    for recipient in email_receivers:
        print(f"Sending email to {recipient}")
        message = MIMEMultipart('alternative')
        message['From'] = EMAIL_SENDER_ACCOUNT
        message['To'] = recipient
        message['Subject'] = email_subject
        message.attach(MIMEText(email_body))
        text = message.as_string()
        server.sendmail(EMAIL_SENDER_ACCOUNT,recipient,text)
    #All emails sent, log out.
    server.quit()

if __name__=='__main__':

    #Email Content
    email_receivers = ["oyediran.hafiz@gmail.com, psubedi3@huskers.unl.edu"] #enter the list of recepient emails
    email_subject = "Smart IoT Notification"
    email_body = """testing testing testing"""

    send_email(email_subject, email_receivers, email_body)
