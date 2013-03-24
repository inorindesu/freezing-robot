-- This script have an inconsistency between parameter count indicated
-- in paramCount and real parameter count of calc function.
--
-- Try loading it with the program and see what will happen!

paramCount = 1
calc = function(a, b)
     return a + b
end
