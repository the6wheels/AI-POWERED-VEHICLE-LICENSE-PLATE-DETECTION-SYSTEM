import subprocess
import sys

# Wait for input
user_input = input("Enter 1 to say hello or any other key to be kicked out: ")

# Check the input
if user_input == "1":
    # Open new terminal window and execute sc1.py
    subprocess.Popen(['gnome-terminal', '--', 'bash', '-c', 'python sc1.py'])
else:
    # Open new terminal window and execute sc2.py
    subprocess.Popen(['gnome-terminal', '--', 'bash', '-c', 'python sc2.py'])

# Exit the program
sys.exit()

