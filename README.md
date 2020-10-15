# HTTP-Proxy
HTTP-Proxy in C using Princeton's assignment as a template.

Assignment under Prof. Manikantan Srinivasan
================================================================

    README file for CS3205 Assignment - HTTP Proxy

           Name: Akash N A and Vamsi KV
           Roll: CS17B020 and CS17B045 (Respectively)
 
================================================================

Instructions to run:
The code to be used is contained in "proxy.c".

1.Testing using telnet:

    Run on two different terminals the following (in order):
    
    a] Terminal-1: run the command :-
                    |>./proxy <port-no of proxy>
    
    b] Terminal-2: run the command :-
                    |>telnet localhost <port-no of proxy>
                    |Trying 127.0.0.1...
                    |Connected to localhost.
                    |Escape character is '^]'.
                    |GET <URL> HTTP/1.0\r\n\r\n                   ------> This input from the user.
    Make note that the last command on Terminal-2 will not work as it is. Its is best to copy the next two lines along with the url
    from the "paster" file. Contact author for more information.
    
2.Testing using the testing python scripts provided:

    a] "proxy_tester.py": run the following command on a system enabled with python2 (and python2 only)-
                    python proxy_tester.py proxy <port-no of proxy>
                    
    b] "proxy_tester_conc.py": run the following command on a system enabled with python2 (and python2 only)
                                and also has apache benchmarks installed (try the command "ab" on the terminal,
                                it will give the command to install apache benchmarks)-
                    python proxy_tester_conc.py proxy <port-no of proxy>
                    
If there are any errors in connecting to the sockets (or any other), code will generate error messages.
Invalid test cases - HTTP error 400 and 501 have also been enabled.

A common occurance while testing might be "Error in binding with socket <no>". Try using a different socket or 
try waiting for a while before using the same socket(so that it can be cleared). To avoid this, try avoiding 
abnormal termination of proxy. Emphasis on this while using telnet to test. Use Ctrl-C to terminate the proxy.

