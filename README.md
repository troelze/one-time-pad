# one-time-pad
This program will encrypt and decrypt plaintext into ciphertext, using a key. The 27 characters allowed are the 26 capital letters, and the space character ( ). Modulo 27 is the operation used to encrypt and decrypt. 

### otp_enc_d
  
This program runs in the background, and listens on a particular port/socket that is assigned when first run. Supports up to five concurrent socket connections running at the same time. 

    otp_enc_d listening_port

    $ otp_enc_d 57171 &

All error text will output to stderr.

### otp_enc
  
This program connects to otp_enc_d, and asks it to perform a one-time pad style encryption as detailed above. 

    otp_enc plaintext key port

In this syntax, plaintext is the name of a file in the current directory that contains the plaintext you wish to encrypt. Similarly, key contains the encryption key you wish to use to encrypt the text. Finally, port is the port that otp_enc should attempt to connect to otp_enc_d on.

When otp_enc receives the ciphertext back from otp_enc_d, it will output to stdout. otp_enc can be launched in any of the following methods:

    $ otp_enc myplaintext mykey 57171
    $ otp_enc myplaintext mykey 57171 > myciphertext
    $ otp_enc myplaintext mykey 57171 > myciphertext &

If otp_enc receives key or plaintext files with ANY bad characters in them, or the key file is shorter than the plaintext, then it should terminate, send appropriate error text to stderr, and set the exit value to 1.

### otp_dec_d
  
This program performs exactly like otp_enc_d, in syntax and usage. In this case, however, otp_dec_d will decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext again to otp_dec.

### otp_dec

Similarly, this program will connect to otp_dec_d and will ask it to decrypt ciphertext using a passed-in ciphertext and key, and otherwise performs exactly like otp_enc, and is runnable in the same three ways. 

### keygen

This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods. 

The syntax for keygen is as follows:

    keygen keylength
