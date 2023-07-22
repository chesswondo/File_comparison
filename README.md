# File comparison

File comparison. The client and server initially store identical text files.
Files may change over time (using any text editor), but they will be quasi-identical if:
1) The uppercase and lowercase characters are identical and
2) All "white" fields (one or more spaces) are equivalent to one space.
   
On command from the client, comparison procedure is performed and then the user on the client receives a list of rows in which quasi-identity is violated.
