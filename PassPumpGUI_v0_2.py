#!/usr/bin/env python
#
# GUI to interface with the PasswordPump
#
# Copyright 2019
# Daniel Murphy
#
from Tkinter import *
import tkinter.ttk as ttk
import serial
import serial.tools.list_ports
from serial.tools.list_ports import comports
import argparse

window = Tk()
window.title("PasswordPump Edit Credentials")
window.geometry('375x500')

lbl_port = Label(window, text="Port", anchor=E, justify=RIGHT, width=10)
lbl_port.grid(column=0, row=0)

lbl_acct = Label(window, text="Account", anchor=E, justify=RIGHT, width=10)
lbl_acct.grid(column=0, row=1)

lbl_user = Label(window, text="User Name", anchor=E, justify=RIGHT, width=10)
lbl_user.grid(column=0, row=2)

lbl_pass = Label(window, text="Password", anchor=E, justify=RIGHT, width=10)
lbl_pass.grid(column=0, row=3)

lbl_style = Label(window, text="Style", anchor=E, justify=RIGHT, width=10)
lbl_style.grid(column=0, row=4)

lbl_help = Label(window, text="Instructions", anchor=W, justify=CENTER, width=10)
lbl_help.grid(column=1, row=6)

txt_acct = Entry(window, width=40)
txt_acct.grid(column=1, row=1)

txt_user = Entry(window, width=40)
txt_user.grid(column=1, row=2)

txt_pass = Entry(window, width=40)
txt_pass.grid(column=1, row=3)

txt_style = Entry(window, width=40)
txt_style.grid(column=1, row=4)

txt_acct.config(state='disabled')
txt_user.config(state='disabled')
txt_pass.config(state='disabled')
txt_style.config(state='disabled')

def clickedAcct():
    resAcct = txt_acct.get()
    s.write(resAcct + '\n')
    txt_acct.config(state='disabled')
#    window.after(100, poll)
    directions = """On the PasswordPump long click
to accept the entered account 
name, then short click on Edit
Username, then enter the 
username in the text box 
above.  Then hit return or 
click on Submit."""
    txt_dir.delete('1.0', END)
    txt_dir.insert(END, directions)
    print directions
    window.update()
    poll()

def clickedUser():
    resUser = txt_user.get()
    s.write(resUser + '\n')
    txt_user.config(state='disabled')
#    window.after(100, poll)
    directions = """On the PasswordPump long click
to accept the entered user 
name, then short click on 
Edit Password, then enter the 
password in the text box 
above.  Then hit return or 
click on Submit."""
    txt_dir.delete('1.0', END)
    txt_dir.insert(END, directions)
    print directions
    window.update()
    poll()

def clickedPass():
    resPass = txt_pass.get()
    s.write(resPass + '\n')
    txt_pass.config(state='disabled')
#    window.after(100, poll)
    directions = """On the PasswordPump long click
to accept the entered 
password, then short click on 
Indicate Style, then enter the
style in the text box above.
Style controls whether or not
a carriage return or a tab is
sent between the sending of
the username and the 
password. Enter 0 for 
carriage return, 1 for tab 
between username and password
when both are sent. Then hit
return or click on Submit. 
The style can also be entered
via the rotary encoder; turn
the encoder to select 0 or 1,
then short click and then 
long click. """
    txt_dir.delete('1.0', END)
    txt_dir.insert(END, directions)
    print directions
    window.update()
    poll()

def clickedStyle():
    resStyle = txt_style.get()
    txt_style.config(state='disabled')
    s.write(resStyle + '\n')
    txt_style.config(state='disabled')
    directions = """On the PasswordPump long click
to accept the entered style.
On the PasswordPump long 
click to finish entering the
credentials, then close this
application by clicking on
Exit."""
    txt_dir.delete('1.0', END)
    txt_dir.insert(END, directions)
    window.update()
    print directions

def clickedClose():
    if s.is_open:
        s.close()
    sys.exit(1)

def clickedOpen():
    global s
    s = serial.Serial(port, 38400)
    btn_open.config(state='disabled')
    btn_close.config(state='normal')
    directions = """On the PasswordPump navigate 
to Add Account and short 
click. Then short click on
Account Name, enter the
account name in the Account
Name text box above, and hit
return or click on Submit."""
    txt_dir.delete('1.0', END)
    txt_dir.insert(END, directions)
    print directions
    window.update()
    poll()

def poll():
    attribute = s.readline()
    print attribute
    if int(attribute) == 8:    #account
        txt_acct.config(state='normal')
        txt_user.config(state='disabled')
        txt_pass.config(state='disabled')
        txt_style.config(state='disabled')
        txt_acct.focus()
        btn_acct.config(state='normal')
        btn_user.config(state='disabled')
        btn_pass.config(state='disabled')
        btn_style.config(state='disabled')
        window.bind('<Return>', (lambda e, btn_acct=btn_acct: btn_acct.invoke()))
        print('in 8')
    elif int(attribute) == 5:  #username
        txt_acct.config(state='disabled')
        txt_user.config(state='normal')
        txt_pass.config(state='disabled')
        txt_style.config(state='disabled')
        txt_user.focus()
        btn_acct.config(state='disabled')
        btn_user.config(state='normal')
        btn_pass.config(state='disabled')
        btn_style.config(state='disabled')
        window.bind('<Return>', (lambda e, btn_user=btn_user: btn_user.invoke()))
        print('in 5')
    elif int(attribute) == 6:  #password
        txt_acct.config(state='disabled')
        txt_user.config(state='disabled')
        txt_pass.config(state='normal')
        txt_style.config(state='disabled')
        txt_pass.focus()
        btn_acct.config(state='disabled')
        btn_user.config(state='disabled')
        btn_pass.config(state='normal')
        btn_style.config(state='disabled')
        window.bind('<Return>', (lambda e, btn_pass=btn_pass: btn_pass.invoke()))
        print('in 6')
    elif int(attribute) == 4:  #style
        txt_acct.config(state='disabled')
        txt_user.config(state='disabled')
        txt_pass.config(state='disabled')
        txt_style.config(state='normal')
        txt_style.focus()
        btn_acct.config(state='disabled')
        btn_user.config(state='disabled')
        btn_pass.config(state='disabled')
        btn_style.config(state='normal')
        window.bind('<Return>', (lambda e, btn_style=btn_style: btn_style.invoke()))
        print('in 4')

def serial_ports():
    return comports()

def on_select(event=None):
    global port
    port = cb.get()
    # get selection directly from combobox
    print("comboboxes: ", cb.get())

btn_acct = Button(window, text="Submit", command=clickedAcct)
btn_acct.grid(column=2, row=1)

btn_user = Button(window, text="Submit", command=clickedUser)
btn_user.grid(column=2, row=2)

btn_pass = Button(window, text="Submit", command=clickedPass)
btn_pass.grid(column=2, row=3)

btn_style = Button(window, text="Submit", command=clickedStyle)
btn_style.grid(column=2, row=4)

btn_close = Button(window, text=" Exit ", command=clickedClose)
btn_close.grid(column=2, row=5)

btn_open = Button(window, text="Open Port", command=clickedOpen)
btn_open.grid(column=0, row=5)

btn_acct.config(state='disabled')
btn_user.config(state='disabled')
btn_pass.config(state='disabled')
btn_style.config(state='disabled')
btn_close.config(state='disabled')

txt_dir = Text(window, height=19, width=30, relief=FLAT, background="light grey")
txt_dir.grid(column=1, row=8)
txt_dir.config(state=NORMAL)
txt_dir.delete('1.0', END)
directions = """After selecting the port click
on the Open Port button to 
open the port."""
txt_dir.insert(END, directions)

ports = []
for n, (port, desc, hwid) in enumerate(sorted(comports()), 1):
    ports.append(port)

cb = ttk.Combobox(window, values=ports, justify=RIGHT, width=37)
cb.grid(column=1, row=0)
cb.bind('<<ComboboxSelected>>', on_select)

window.mainloop()
