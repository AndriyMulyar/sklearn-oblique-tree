Documentation : 
  The documentation in OC1 source code comprises of file headers, 
  module headers and in-code comments.  
 
  File Headers : Each .C file starts with file header that gives a part
                 or all of the following information.

  1. Copyright information
  2. Contact E-mail address
  3. File Name
  4. Author
  5. Month of last modification 
  6. Modules (procedures or functions) in the file
  7. Files that contain modules that the modules in this file use
  8. Files that contain modules that use the modules in this file
  9. Remarks


  Module Headers : Each procedure or function is preceded by a module header
                   that gives a part or all of the following information.
 
  1. Module name 
  2. A brief description of what the module does.
  3. Formal parameters the module takes
  4. What the module returns on successful completion
  5. List of modules that are called from this module.
     If one (or some) of the above are not physically in the same file as
     the current module, the file name is mentioned in paranthesis after
     the module name.
  6. List of modules that call this module
     If one (or some) of the above are not physically in the same file as
     the current module, the file name is mentioned in paranthesis after
     the module name.
  7. Description of any important variables or constants used in the module.
  8. Additional Remarks


  In-code comments : File headers and module headers comprise of a very 
                     sizeable portion of the OC1 documentation. In addition,
                     there are remarks in places where the author felt (!!) they
                     were needed to better understand the code segments.

Code :

  1. Variables are named (occasionally resulting in a lot of extra typing)
     to reflect the quantities they store. For example, "max_no_of_random_
     perturbations" is the variable (in mktree.c) to store the maximum number
     of random perturbations tried when stuck in a local minimum. The question
     of whether this convention is better than, say, naming the above variable
     as "max_rp", is subjective. But we feel that it does serve the purpose
     of understandability.

  2. The same applies to most of the module names and file names.
 

