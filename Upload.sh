#!/bin/bash
cd /home/pipenooyimsai/Documents/Project_WPNS-IoT

git add .

commit_message="Auto-update: $(date '+%Y-%m-%d %H:%M:%S')"

git commit -m "$commit_message"

git push origin main
