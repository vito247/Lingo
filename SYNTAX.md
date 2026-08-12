# Lingo Syntax 

## Import
```Import [Library Path] (As) [(Namespace)]```: Imports a library. If ```As [Namespace]``` is omitted, the library's file name is used as the namespace.

## Display  
```Display [Object]``` : Displays an object to console.  

## Variable  
```Var [type] [name] = [value]``` : Creates variable and set its value.  
```Const [type] [name] = [value]``` : Creates constant and set its value.  
```Set [Identifier] = [value]``` : Sets value of variable.  

## Conditional Statements
```
If [condition],  
Do  
    [code]  
```
The "Do" block is executed when the [condition] is true.  
  
```
Else,  
Do  
    [code]  
```
The "Do" block is executed when the [condition] is false. Required "If".  

## Repeat
```
Repeat [number]
Do  
    [code]
 ```  
The "Do" block is executed [number] times.

## Function
```
Function [name] -> [Return Type]  
Input  
    [Variable(s)]  
Do  
    [code]  
    Throw [Object]
```  
Creates function, this function can call with its name.  
"Input" block, set parameters of this function.  
The "Do" block is executed when this function has called.  
The "Throw" code returns value of this function, type of throwing value must match the [Return Type] of function.  

## Examples  
```
Var num age = 20  
  
If age more 18,  
Do  
    Display "Adult"  
Else,
Do
    Display "Not adult"
```
```
Function add -> num  
Input  
    Var num a  
    Var num b  
Do  
    Throw a + b  
  
Display add(5, 3)  
```


