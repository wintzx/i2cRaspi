git config --global user.email "you@example.com"
git config --global user.name "Your Name"

touch README.md
git init
git add README.md
git commit -m "first commit"
git remote add origin https://github.com/wintzx/i2cRaspi.git
git push -u origin master
