#!/usr/bin/python

import Tkinter as Tk
import tkFileDialog, tkMessageBox, ScrolledText
import subprocess
import os, glob, errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

class StatusBar(Tk.Frame):
    def __init__(self, master):
		Tk.Frame.__init__(self, master)
		self.text = ScrolledText.ScrolledText(self, bd=1, relief=Tk.SUNKEN, height = 4, state = Tk.NORMAL, wrap = Tk.NONE)
		self.text.pack(fill=Tk.X)
	
    def set(self, format, *args):
		self.text.config(state = Tk.NORMAL)
		self.text.insert(Tk.END, format % args)
		self.text.see('end')
		self.text.config(state = Tk.DISABLED)
		self.text.update_idletasks()
		

    def clear(self):
		self.text.config(state = Tk.NORMAL)
		self.text.delete(1.0, Tk.END)
		self.text.config(state = Tk.DISABLED)
		self.text.update_idletasks()

class App:
	def __init__(self, master):
		frame = Tk.Frame(master)
		frame.pack()
		
		input = Tk.Frame(frame)
		input.grid(row = 1, column = 0)
		Tk.Label(input, text = 'Input:').grid(row = 0, sticky=Tk.E)
		Tk.Label(input, text = 'Output:').grid(row = 1, sticky=Tk.E)
		self.in_dir = Tk.StringVar()
		self.in_dir.set("/data/blinded/")
		self.out_dir = Tk.StringVar()
		self.out_dir.set(os.getcwd())
		self.in_entry  = Tk.Entry(input, width = 64, textvariable = self.in_dir)
		self.in_button = Tk.Button(input, text = "...", command = self.find_in)
		self.out_entry = Tk.Entry(input, width = 64, textvariable = self.out_dir)
		self.out_button = Tk.Button(input, text = "...", command = self.find_out)
		self.in_entry.grid(row = 0, column = 1)
		self.in_button.grid(row = 0, column = 2)
		self.out_entry.grid(row = 1, column = 1)
		self.out_button.grid(row = 1, column = 2)
		
		type = Tk.Frame(frame)
		type.grid(row = 0, column = 0)
		self.type = Tk.IntVar()
		self.type.set(1)
		Tk.Radiobutton(type, text = "Single scan", variable = self.type, value = 0).grid(row = 0, column = 0, sticky = Tk.W)
		Tk.Radiobutton(type, text = "Single subject", variable = self.type, value = 1).grid(row = 0, column = 1, sticky = Tk.W)
		Tk.Radiobutton(type, text = "Multiple subjects", variable = self.type, value = 2).grid(row = 0, column = 2, sticky = Tk.W)
		
		options = Tk.Frame(frame)
		options.grid(row = 2, column = 0)
		self.spm_scale = Tk.IntVar()
		self.spm_scale.set(1)
		Tk.Checkbutton(options, text = "Scale for SPM", variable = self.spm_scale).grid(row = 0, column = 0, sticky = Tk.W)
		self.corax = Tk.IntVar()
		self.corax.set(0)
		Tk.Checkbutton(options, text = "Human Coronal/Axial", variable = self.corax).grid(row = 0, column = 1, sticky = Tk.W)
		self.embed_procpar = Tk.IntVar()
		self.embed_procpar.set(1)
		Tk.Checkbutton(options, text = "Embed procpar", variable = self.embed_procpar).grid(row = 0, column = 2, sticky = Tk.W)
		self.gz = Tk.IntVar()
		self.gz.set(0)
		Tk.Checkbutton(options, text = "Compress", variable = self.gz).grid(row = 0, column = 3, sticky = Tk.W)
		self.ignore_scout = Tk.IntVar()
		self.ignore_scout.set(1)
		Tk.Checkbutton(options, text = "Ignore scouts", variable = self.ignore_scout).grid(row = 0, column = 4, sticky = Tk.W)
		self.echo_mode = Tk.IntVar()
		self.echo_mode.set(-1)
		Tk.Label(options, text = "Multi-echo:").grid(row = 1, column = 0, sticky = Tk.E)
		Tk.Radiobutton(options, text = "All", variable = self.echo_mode, value = -1).grid(row = 1, column = 1, sticky = Tk.W)
		Tk.Radiobutton(options, text = "Sum", variable = self.echo_mode, value = -2).grid(row = 1, column = 2, sticky = Tk.W)
		Tk.Radiobutton(options, text = "Mean", variable = self.echo_mode, value = -3).grid(row = 1, column = 3, sticky = Tk.W)
		Tk.Radiobutton(options, text = "Select:", variable = self.echo_mode, value=0).grid(row = 1, column = 4, sticky = Tk.W)
		self.echo_select = Tk.Spinbox(options, from_ = 0, to=999999, width = 8) # Hopefully that is high enough!
		self.echo_select.grid(row = 1, column = 5, sticky = Tk.W)
		
		self.go_button = Tk.Button(options, text = "Convert", command = self.go)
		self.go_button.grid(row = 0, column = 6, rowspan = 2, columnspan = 2, sticky=Tk.N+Tk.S+Tk.E+Tk.W)
		self.status_bar = StatusBar(frame)
		self.status_bar.set("Ready.\n")
		self.status_bar.grid(row = 3, sticky=Tk.W + Tk.E)
		self.status_bar.grid_propagate(False)
		
		menu = Tk.Menu(root)
		root.config(menu=menu)
		filemenu = Tk.Menu(menu)
		menu.add_cascade(label="File", menu=filemenu)
		filemenu.add_command(label="Exit", command=frame.quit)
		master.title("FDF to Nifti Conversion Tool")
		self.master = master
		
	def find_in(self):
		self.in_dir.set(tkFileDialog.askdirectory(initialdir = self.in_dir.get(), mustexist = True))
	
	def find_out(self):
		self.out_dir.set(tkFileDialog.askdirectory(initialdir = self.out_dir.get(), mustexist = False))
	
	def runCommand(self, inpath, outpath):
		command = 'fdf2nii -v '
		if self.spm_scale.get():
			command = command + '-s 10.0 '
		
		if self.corax.get():
			command = command + '-c '

		if self.embed_procpar.get():
			command = command + '-p '
		
		if self.gz.get():
			command = command + '-z '
		
		if self.echo_mode.get() >= 0:
			command = command + '-e ' + self.echo_select.get() + ' '
		else:
			command = command + '-e ' + str(self.echo_mode.get()) + ' '
		
		command = command + '-o ' + outpath + ' '
		command = command + inpath
		
		self.status_bar.set("Running command: " + command + '\n')
		p = subprocess.Popen(command,
							 shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		for line in iter(p.stdout.readline, ''):
			self.status_bar.set(line)
			self.master.update_idletasks()
	
	def findScans(self, inpath):
		scans = glob.glob(inpath + '/*.img')
		list = ''
		for s in scans:
			if (self.ignore_scout.get() and (os.path.basename(s).lower().find("scout") != -1)):
				pass
			else:
				list = list + s + ' '
		return list
	
	def go(self):
		self.status_bar.clear()
		self.status_bar.set("Starting.\n")
		self.master.config(cursor = "watch")
		self.master.update_idletasks()
		outpath = self.out_dir.get()
		
		if (self.type.get() == 0):
			(inpath, inext) = os.path.splitext(os.path.normpath(self.in_dir.get()))
			(indir, inbase) = os.path.split(os.path.normpath(inpath))
			if inext != ".img":
				tkMessageBox.showwarning("Wrong Extension", "You must select the .img folder when converting a single image.")
				self.status_bar.set("Wrong extension on directory: " + self.in_entry.get() + "\n")
			else:
				self.runCommand(inpath + inext, outpath + '/')
		elif (self.type.get() == 1):
			inpath = os.path.normpath(self.in_dir.get())
			(indir, inbase) = os.path.split(os.path.normpath(inpath))
			scans = self.findScans(inpath)
			if scans:
				outpath = outpath + '/' + inbase + '/'
				mkdir_p(outpath)
				self.runCommand(scans, outpath)
			else:
				tkMessageBox.showwarning("No scans", "No scans to convert were found directory: " + inpath)
				self.status_bar.set("No scans to convert were found directory: " + inpath + "\n")
		elif (self.type.get() == 2):
			inpath = os.path.normpath(self.in_dir.get())
			subjects = glob.glob(inpath)
			for subj in subjects:
				(subjdir, subjbase) = os.path.split(subj)
				subjscans = self.findScans(subj)
				if subjscans:
					subjout = outpath + '/' + subjbase + '/'
					mkdir_p(subjout)
					self.runCommand(subjscans, subjout)
				else:
					tkMessageBox.showwarning("No scans", "No scans to convert were found in directory: " + subj)
					self.status_bar.set("No scans to convert were found directory: " + subj + "\n")
		self.status_bar.set("Finished all conversions.\n")
		self.master.config(cursor = "")

root = Tk.Tk()
app = App(root)
root.resizable(0, 0)
root.mainloop()
