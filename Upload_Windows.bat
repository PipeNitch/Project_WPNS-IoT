cd /d D:\Documents\Project\WPNS-IoT

git add .

set commit_message=Update: %date% %time:~0,8% from Windows

git commit -m "%commit_message%"

git push -u origin main