# Lingo Syntax 

```Display [Object]``` : Displays object to console.  

```Var [type] [name] = [value]``` : Create variable and set its value.  
```Const [type] [name] = [value]``` : Create constant and set its value.  
```Set [Identifier] = [value]``` : Set variable value.  

```If [condition],``` : If [condition] is true, run [code] within "Do" block and indent(s).    
```Do```  
```  [code]```  
```Else,``` : If [condition] is false, run [code] within "Do" block and indent(s).  
```Do```  
```  [code]```  

```Repeat [number]``` : Repeat code [number] times within "Do" block and indent(s).  
```Do```  
```  [code]```  

```Function [name] -> [Return Type]``` : Create function, this function can call with its name.  
```Input``` : "Input" block, set parameters of this function.  
```  [Variable(s)]```  
```Do``` : "Do" block, when this function has called, run [code].  
```  [code]```  
```  Throw [Object]``` : Return value of this function, it must be correct with [Return Type] of this function.  
