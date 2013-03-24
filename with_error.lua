-- This script will trigger runtime error when running.
-- (at 'a + nil')
--
-- Try loading it with the program and see what will happen!

paramCount = 1
calc = function(a)
     return a + nil
end
