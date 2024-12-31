#!/bin/bash
cd /home/pipenooyimsai/Documents/Projects/Project_WPNS-IoT

git add .

commit_message="Update: $(date '+%Y-%m-%d %H:%M:%S') from Linux"

git commit -m "$commit_message"

git push -u origin main
