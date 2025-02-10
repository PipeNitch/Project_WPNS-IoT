cd /d D:\Documents\Project\WPNS-IoT

git add .

for /f %%i in ('hostname') do set device_name=%%i

set commit_message=Update: %date% %time:~0,8% from Windows (%device_name%)

git commit -m "%commit_message%"

git push -u origin main --force
