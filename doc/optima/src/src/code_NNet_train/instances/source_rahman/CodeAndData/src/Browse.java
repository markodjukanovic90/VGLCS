





import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Browse{ 
//extends JFrame implements ActionListener {

/*
JButton button, button1;
JTextField field;

Reader reader=new Reader();
String bnames[]={ "Seq-Ic-LCS FA","Seq-Ic-LCS CC", "Seq-Ec-LCS FA", 
		          "Seq-Ec-LCS ZG",  "Str-Ec-LCS FA", "Str-Ec-LCS HYA"};

JRadioButton LCS_Choice[]=new JRadioButton[bnames.length]; 

public Browse () {
this.setLayout(null);

button = new JButton("Browse");
field = new JTextField();
button1=new JButton("b1");

field.setBounds(30, 50, 200, 25);
button.setBounds(240, 50, 100, 25);
this.add(field);
this.add(button);

ButtonGroup buttonGroup = new ButtonGroup();


	JPanel cpanel = new JPanel();
	cpanel.setBounds(60, 85, 260, 150);
	cpanel.setLayout(new GridLayout(3,1));
	
	
	for (int i=0; i< bnames.length; i++) 
	{
		LCS_Choice[i] = new JRadioButton( bnames[i]);
		LCS_Choice[i].addActionListener(this);
		buttonGroup.add(LCS_Choice[i]);
		cpanel.add(LCS_Choice[i]);
	}

	this.add(cpanel);
	
button.addActionListener(this);
setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
}

public void actionPerformed(ActionEvent e) {
  
	Object source = e.getSource();
	if(source==button){
	   Chooser frame = new Chooser();
       field.setText(frame.fileName);
       this.reader= frame.reader;
    }
	
	if(source==LCS_Choice[0]){
		  FiniteAutomata F=new FiniteAutomata();
		  F.Seq_Ic(reader);
		  
	    }
	if(source==LCS_Choice[1]){
		SeqIc r= new SeqIc();
		   //  r.DPmethod2(reader.inputStr);
		    if(reader.totalStr.size()==3)
		       r.DP1(reader.totalStr);
		    else if(reader.totalStr.size()==4)
		       r.DP2(reader.totalStr);
		    else if(reader.totalStr.size()==5)
		       r.DP3(reader.totalStr);
		    else
		       r.DP4(reader.totalStr);
	    }
	
	if(source==LCS_Choice[2]){
		FiniteAutomata F=new FiniteAutomata();
		 F.Seq_Ec(reader); // ovo ovdje kopirati u main funkciju
	    }
	
	if(source==LCS_Choice[3]){
		   RLCS r= new RLCS();
		if(reader.totalStr.size()==3)
		       r.DPmethod2(reader.totalStr);
		    else if(reader.totalStr.size()==4)
		       r.DPmethod3(reader.totalStr);
		    else if(reader.totalStr.size()==5)
		       r.DPmethod5_32(reader.totalStr);
		    else
		       r.DPmethod(reader.totalStr);
		  
	    }
	
	if(source==LCS_Choice[4]){
		FiniteAutomata F=new FiniteAutomata();
		 F.Str_Ec(reader);
	    }
	
	if(source==LCS_Choice[5]){
		StrEc r=new  StrEc();
		  if(reader.totalStr.size()==3)
		    r.StrAlgo1(reader.totalStr);
		  else if(reader.totalStr.size()==4)
			  r.StrAlgo2(reader.totalStr);
		  else if(reader.totalStr.size()==5)
			  r.StrAlgo3(reader.totalStr);
		  else
			  r.StrAlgo4(reader.totalStr);
	    }
	
	  
} 
*/ 

public static void writeToFile(String filePath, String content) {
	try {
		// Convert the string content to bytes
		byte[] contentBytes = content.getBytes();

		// Create a Path object from the file path string
		Path path = Paths.get(filePath);

		// Write the bytes to the file
		Files.write(path, contentBytes);

		System.out.println("String successfully written to the file: " + filePath);
	} catch (IOException e) {
		// Handle potential IO exceptions (e.g., file not found, permissions issues)
		e.printStackTrace();
	}
}

public static void main(String args[]) {
 
   System.out.println(args.length);
   String fileName=args[0];
   String fileOut=args[1];
   int algorithm = Integer.parseInt(args[2]);
   // write-in file
   Reader reader=new Reader();
   reader.readInput(fileName);
   reader.run(); // read data into data structures
   
   if(algorithm==0) // Automaton approach 
   {   
	   FiniteAutomata F=new FiniteAutomata();
       F.Seq_Ec(reader); // ovo ovdje kopirati u main funkciju
       System.out.println("Out report");
        //out file 
        //System.out.println(F.out_report);
       writeToFile(fileOut, F.out_report);
   }else{ // DP approach
 
	   RLCS r = new RLCS();
       if(reader.totalStr.size()==3) // run a method depending on a number of (all) input strings 
          r.DPmethod2(reader.totalStr); 
       else if(reader.totalStr.size()==4) 
          r.DPmethod3(reader.totalStr);
       else if(reader.totalStr.size()==5)
          r.DPmethod5_32(reader.totalStr);
       else
          r.DPmethod(reader.totalStr);
       
	   writeToFile(fileOut, r.output_file);

   }
   // WRITE INTO A FILE 
   /*Browse frame = new Browse ();
   frame.setSize(400, 300);
   frame.setLocation(300, 100);
   frame.setVisible(true);
   // add this:
   
   
   */  

 
}  
};

/*class Chooser extends JFrame {

JFileChooser chooser;
String fileName;
Reader reader=new Reader();

		public Chooser() {
		chooser = new JFileChooser();
		int r = chooser.showOpenDialog(new JFrame());
			if (r == JFileChooser.APPROVE_OPTION) {
			     fileName = chooser.getSelectedFile().getPath();
			     reader.readInput(fileName);
			     reader.run();
			     //System.out.println((reader.concatInput()).length());
			   }
		  }
 }*/
