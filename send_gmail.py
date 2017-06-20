import smtplib
from email.mime.image import MIMEImage
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart #attachments
to = 'chempen@g.ucla.edu'
gmail_user = 'catherinemh96@gmail.com'
gmail_pwd = 'hempen96'
smtpserver = smtplib.SMTP("smtp.gmail.com",587)
smtpserver.ehlo()
smtpserver.starttls()
smtpserver.ehlo
smtpserver.login(gmail_user, gmail_pwd)
header = 'To:' + to + '\n' + 'From: ' + gmail_user + '\n' + 'Subject:testing_edison \n'
print header
#msg = header + '\n Sent from my Intel Edison \n\n'
msg = MIMEMultipart()
msg['From'] = gmail_user
msg['To'] = to
msg['Subject']='Patient Data'
#attach plots:
fp = open('patient_time_series.csv', 'rb')
data = MIMEText(fp.read()) #image format
fp.close()
msg.attach(data)
#attach .csv files with patient data:

smtpserver.sendmail(gmail_user, to, msg.as_string())
print 'done!'
smtpserver.close()
