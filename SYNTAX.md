# Lingo Syntax 

Display [Object] : Displays object to console.  
Var [type] [name] = [value] : Create variable and set its value. 
Const [type] [name] = [value] : Create constant and set its value.  
Set [Identifier] = [value] : Set variable value.  
If [condition] : If [condition] is true, run [code] with "Then" and indent(s).    
  Then [code]  
Else : If [condition] is false, run [code] with "Then" and indent(s).  
  Then [code]  
Repeat [number] : Repeat code [number] times within "What" block and indent(s).  
What  
  [code]  
Function [name] -> [Return Type] : Create function, this function can call with its name.  
Input : "Input" block, set parameters of this function.  
  [Variable(s)]  
What : "What" block, when this function has called, run [code].  
  [code]  
  Throw [Object] : Return value of this function, it must be correct with [Return Type] of this function.  
