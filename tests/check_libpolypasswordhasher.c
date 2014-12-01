/* Check libpolypasswordhasher core, no shielded accounts and no isolated bytes 
 *
 * This suite is designed to test all of the core functionalities of the 
 * libpolypasswordhasher module without its extensions. 
 *
 * @author  Santiago Torres
 * @date    10/03/2014
 * @license MIT
 */


#include<check.h>
#include"libgfshare.h"
#include"libpolypasswordhasher.h"
#include<stdlib.h>
#include<strings.h>






// we are going to check secrets check for proper input
START_TEST(test_generate_pph_secret_input_sanity) {

  uint8 *secret, secret_integrity[DIGEST_LENGTH];

  // check 0 values, remember that without at least one byte of each
  // it is imposible to verify the secret is a valid one
  secret = generate_pph_secret(NULL);
  ck_assert(secret == NULL);

  // now check for a valid construction.
  secret = generate_pph_secret(secret_integrity);
  ck_assert(secret != NULL);

  free(secret);
}END_TEST





// we are going to verify that the check_secret function does sanitate input
START_TEST(test_check_pph_secret_input_sanity) {


  // we are not going to provide a valid secret for this function, so we 
  // don't care about the return value for invalid secrets, instead, we will 
  // generate a bogus stream of uninitialized data.
  uint8 secret[DIGEST_LENGTH], secret_integrity[DIGEST_LENGTH];
  unsigned int stream_length = DIGEST_LENGTH/2;
  unsigned int hash_bytes = DIGEST_LENGTH/2;
  PPH_ERROR error;

  // now check for NULL pointers in the secret
  error = check_pph_secret(NULL, secret_integrity);
  ck_assert(error == PPH_BAD_PTR);

  error = check_pph_secret(secret, NULL);
  ck_assert(error == PPH_BAD_PTR);

  //finally, do a good initialization, and expect something either right or
  //wrong, but signals of a good computation.
  error = check_pph_secret(secret, secret_integrity);
  ck_assert(error == PPH_ERROR_OK || error == PPH_SECRET_IS_INVALID);

}END_TEST





// we check that a secret generated by the generate_pph_secret function does
// produce a valid pph_secret and that such secret is correctly verified by
// the check_pph_secret_function
START_TEST(test_generate_and_check_pph_secret_mixed) {

  uint8 *secret;
  uint8 secret_integrity[DIGEST_LENGTH];
  // this time, we expect to get a valid secret, and then modify it so we get
  // an error.
  unsigned int stream_length = DIGEST_LENGTH;
  unsigned int hash_bytes = DIGEST_LENGTH;
  PPH_ERROR error;
  unsigned int i;

  // we will check all of the possible ranges for this
  for(i=1;i<DIGEST_LENGTH;i++){
    // check that the function returns a valid secret.
    secret = generate_pph_secret(secret_integrity);
    ck_assert(secret != NULL);

    // check that with the proper values we get a correct secret check
    error = check_pph_secret(secret, secret_integrity);
    ck_assert(error == PPH_ERROR_OK);

    // check that with some modification to the secret we get a non-valid
    // secret, we will prove this by inverting the first byte. This often
    // fails when the secret-stream length is 1 byte
    secret[0] = ~secret[0]; 
    error = check_pph_secret(secret, secret_integrity);
    ck_assert(error == PPH_SECRET_IS_INVALID);

    free(secret);
  }
}END_TEST





// we test in the core functionality to avoid a threshold of 0, which
// is the only non possible value (everyting bigger would roll over)
START_TEST(test_pph_init_context_wrong_threshold)
{ 
  // a placeholder for the result.
  pph_context *context;
  uint8 threshold = 0; // this is, obviously, the only wrong threshold value
                              
  uint8 isolated_check_bits = 0;// this function is part of the non-isolated bytes
                          
  context = pph_init_context(threshold, isolated_check_bits);

  ck_assert_msg(context == NULL,
      "the context returned upon a wrong threshold value should be NULL");

  // test for over-extension of the threshold value
  threshold =MAX_NUMBER_OF_SHARES+1;
  context = pph_init_context(threshold, isolated_check_bits);

  ck_assert_msg(context == NULL);
  
}
END_TEST





// Test an initialization with proper values, asking for 0 isolated bytes.
START_TEST(test_pph_init_context_no_isolated_check_bits)
{
  
  
  pph_context *context;
  PPH_ERROR error;
  // set the correct threshold and isolated bytes this time
  uint8 threshold = 2;
  uint8 isolated_check_bits = 0;

  //test for a over-extended value for isolated bytes first
  context = pph_init_context(threshold,DIGEST_LENGTH+1);
  ck_assert_msg(context == NULL);  

  context = pph_init_context(threshold,isolated_check_bits);

  ck_assert_msg(context != NULL, "this was a good initialization");
  
  error = pph_destroy_context(context);

  ck_assert_msg(error == PPH_ERROR_OK, 
      "the free function didn't work properly");

}
END_TEST





// We intend to use it to check the correct parsing of the context values
START_TEST(test_create_account_context) {
 
  PPH_ERROR error;

  // sending bogus information to the create user function.
  error = pph_create_account(NULL, "mr.user", strlen("mr.user"), 
      "yessir,verysecure", strlen("yessir,verysecure"), 1);
  
  ck_assert_msg(error == PPH_BAD_PTR, 
      "We should've gotten BAD_PTR in the return value");
  
}
END_TEST





// this test is intended to check correct sanity check on the username field
START_TEST(test_create_account_usernames) {
 
 
  pph_context *context;
  PPH_ERROR error;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                        
  unsigned char username[MAX_USERNAME_LENGTH+1];
  unsigned int i;
  

  // we will simulate a really big username, terminated with a null character
  // to get an incorrect "length" parameter
  for(i=0;i<MAX_USERNAME_LENGTH;i++) {
    username[i] = 'k'; 
  }
  username[MAX_USERNAME_LENGTH] = '\0';

  // initialize a correct context from scratch
  context = pph_init_context(threshold,isolated_check_bits);
  
  // sending bogus information to the create user function.
  error = pph_create_account(context, username, strlen(username),
      "yessir,verysecure", strlen("yessir,verysecure"), 1);  
  ck_assert_msg(error == PPH_USERNAME_IS_TOO_LONG, 
      "We should've gotten USERNAME_IS_TOO_LONG in the return value");


  error = pph_destroy_context(context);
  ck_assert_msg(error == PPH_ERROR_OK, 
      "the free function didn't work properly after failing to add a user");


}
END_TEST





// this test is intended to check correct sanity checks on the password fields
START_TEST(test_create_account_passwords) {
 
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
  unsigned char password[MAX_PASSWORD_LENGTH+1];
  unsigned int i;


  context = pph_init_context(threshold, isolated_check_bits);

  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  
  
 for(i=0;i<MAX_PASSWORD_LENGTH;i++) {
    password[i] = 'k'; // endless string of k's
  }
  password[MAX_PASSWORD_LENGTH] = '\0';
 
  // sending bogus information to the create user function.
  error = pph_create_account(context, "ichooseverylongpasswords",
      strlen("ichooseverylongpasswords"),password,strlen(password),1); 
  ck_assert_msg(error == PPH_PASSWORD_IS_TOO_LONG, 
      "We should've gotten PPH_PASSWORD_IS_TOO_LONG in the return value");
  

  error = pph_destroy_context(context);
  ck_assert_msg(error == PPH_ERROR_OK, 
      "the free function didn't work properly");


}
END_TEST





// this test is intended to check the correct sanity check on the sharenumber
// field
START_TEST(test_create_account_sharenumbers) {
  // as for this version, sharenumbers cannot be wrong due to the nature
  // of the sharenumber variable, but I will leave this test stated in case
  // this ever changes...
  }
END_TEST





// this test is intended to check that a correct account structure is 
// producted, we check the expected hash matches and that we have a two-way
// flow. In other words, that we can derive the hash from the xored hash and 
// vice versa
START_TEST(test_create_account_entry_consistency) {

  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                          
  unsigned char password[] = "verysecure";
  unsigned char username[] = "atleastitry";
  
  // We don't know the salt yet, but we know the password value, upon creating
  // the account, we will replace those x's with the salt values. 
  unsigned char salted_password[] = {'x','x','x','x','x','x','x','x','x',
                                     'x','x','x','x','x','x','x','v','e','r',
                                     'y','s','e','c','u','r','e','\0'};
  uint8 password_digest[DIGEST_LENGTH]; 
  unsigned int i;
  uint8 *digest_result;
  uint8 share_result[SHARE_LENGTH];


  context = pph_init_context(threshold, isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
 
  // create the username. 
  error = pph_create_account(context, username,strlen(username),
      password,strlen(password),1);
  ck_assert_msg(error == PPH_ERROR_OK, 
      "We should've gotten PPH_ERROR_OK in the return value");
 
  // we do this because we assume the username here is a normal string, but 
  // under normal circumstances, we can't assume this.  
  context->account_data->account.username[strlen(username)]='\0'; 
  ck_assert_str_eq(username,context->account_data->account.username);

  // now lets check we can take the digest back from the share
  memcpy(salted_password,context->account_data->account.entries->salt,
      MAX_SALT_LENGTH);
  _calculate_digest(password_digest, salted_password, 
      MAX_SALT_LENGTH + strlen(password));
  digest_result=context->account_data->account.entries->protector_value;


  gfshare_ctx_enc_getshare(context->share_context, 1, share_result);
  _xor_share_with_digest(digest_result, share_result, digest_result, 
      DIGEST_LENGTH);

  // compare the resulting digests to prove they match.
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(password_digest[i]==digest_result[i]);
  }

  // we will check for the existing account error handler now...
  error = pph_create_account(context, username, strlen(username),
      password, strlen(password),1);
  ck_assert_msg(error == PPH_ACCOUNT_EXISTS, 
      "We should've gotten an error since this account repeats");
  

  // finally, check it returns the proper error code if the vault is locked
  // still, we will simulate account locking by unsetting the flag. 
  context->is_bootstrapped = false; 
                           
  
  // we will check for the locked context error now...
  error = pph_create_account(context, "someotherguy", strlen("someotherguy"),
    "came-here-asking-the-same-thing",strlen("came-here-asking-the-same-thing")
    ,1);
  ck_assert_msg(error == PPH_CONTEXT_IS_LOCKED, 
      "We should've gotten an error now that the vault is locked");
 

  error = pph_destroy_context(context);
  ck_assert_msg(error == PPH_ERROR_OK, 
      "the free function didn't work properly");

}
END_TEST





// This test checks for the input of the check_login function, proper error 
// codes should be returned. 
START_TEST(test_check_login_input_sanity) {


  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                          
  unsigned char password[] = "i'mnothere";
  unsigned char username[] = "nonexistentpassword";
  unsigned char too_big_username[MAX_USERNAME_LENGTH+2];
  unsigned char too_big_password[MAX_PASSWORD_LENGTH+2];
  unsigned int i;


  // lets send a null context pointer first
  context=NULL;
  error = pph_check_login(context, username,strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_BAD_PTR, "expected PPH_BAD_PTR");

  // we will send a wrong username pointer now
  context = pph_init_context(threshold, isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  error = pph_check_login(context, NULL, 0, password, 0);
  ck_assert_msg(error == PPH_BAD_PTR, "expected PPH_BAD_PTR");
 
  // do the same for the password 
  error = pph_check_login(context, username, 0, NULL, 0); 
  ck_assert_msg(error == PPH_BAD_PTR, "expected PPH_BAD_PTR");
  
  // now lets create some big usernames and passwords
  for(i=0;i<MAX_USERNAME_LENGTH+1;i++) {
    too_big_username[i]='j';
  }
  too_big_username[i]='\0'; // null terminate our string
  // and query for a login

  error = pph_check_login(context, too_big_username, strlen(too_big_username),
      password, strlen(password));
  ck_assert_msg(error == PPH_USERNAME_IS_TOO_LONG,
      "expected USERNAME_IS_TOO_LONG");

  // let's do the same with the password
  for(i=0;i<MAX_PASSWORD_LENGTH+1;i++) {
    too_big_password[i]='j'; 
  }
  too_big_password[i]='\0'; 

  error=pph_check_login(context, username, strlen(username),
      too_big_password, strlen(too_big_password)); 
  ck_assert_msg(error == PPH_PASSWORD_IS_TOO_LONG,
      "expected PASSWORD_IS_TOO_LONG");
 
  // create an account for a proper account check
  error = pph_create_account(context, username, strlen(username), password,
         strlen(password), 1); 
  ck_assert(error == PPH_ERROR_OK);

  // finally, check it returns the proper error code if the vault is locked
  // still. We set the bootstrapped flag to false to lock the context, and we also
  // know that isolated bytes is 0 and won't provide any login functionality. 
  context->is_bootstrapped = false; 

  error=pph_check_login(context,username,strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_CONTEXT_IS_LOCKED,
     "expected CONTEXT_IS_LOCKED"); 


  pph_destroy_context(context);
}
END_TEST





// This checks for a proper return code when asking for the wrong username
START_TEST(test_check_login_wrong_username) {


  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
  
  unsigned char password[] = "i'mnothere";
  unsigned char username[] = "nonexistentpassword";
  unsigned char anotheruser[] = "0anotheruser";
  unsigned int i;

  
  // setup the context 
  context = pph_init_context(threshold,isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  
  // check with an uninitialized userlist first
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      "expected ACCOUNT_IS_INVALID");

  // add a single user and see how it behaves:
  // 1) add a user
  error = pph_create_account(context, anotheruser, strlen(anotheruser),
      "anotherpassword", strlen("anotherpassword"), 1);
  ck_assert_msg(error == PPH_ERROR_OK, " this shouldn't have broken the test");

  // 2) ask for a user that's not here
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      "expected ACCOUNT_IS_INVALID");
  
  
  // lets add a whole bunch of users and check for an existing one again
  // 1) add a whole new bunch of users:
  for(i=1;i<9;i++) {
    anotheruser[0] = i+48;
    error = pph_create_account(context, anotheruser, strlen(anotheruser),
        "anotherpassword",strlen("anotherpassword"), 1);
    ck_assert_msg(error == PPH_ERROR_OK,
        " this shouldn't have broken the test");
  }

  // 2) ask for a user that's not here
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      "expected ACCOUNT_IS_INVALID");
  
  pph_destroy_context(context);
}
END_TEST





// This checks for a proper return code when providing a wrong password 
START_TEST(test_check_login_wrong_password) {

  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2;
  uint8 isolated_check_bits = 0;
                          
  unsigned char password[] = "i'mnothere";
  unsigned char username[] = "nonexistentpassword";
  unsigned char anotheruser[] = "0anotheruser";
  unsigned int i;


  // setup the context 
  context = pph_init_context(threshold,isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  

  // add a single user and see how it behaves:
  // 1) add a user
  error = pph_create_account(context, username, strlen(username),
      "anotherpassword", strlen("anotherpassword"), 1);
  ck_assert_msg(error == PPH_ERROR_OK, " this shouldn't have broken the test");

  // 2) ask for it, providing wrong credentials
  error = pph_check_login(context, username, strlen(username), password, 
      strlen(password));
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      "expected ACCOUNT_IS_INVALID");
  
  
  // lets add a whole bunch of users and check for an existing one again
  // 1) add a whole new bunch of users:
  for(i=1;i<9;i++) {
    anotheruser[0] = i+48;
    error = pph_create_account(context, anotheruser, strlen(anotheruser),
        "anotherpassword", strlen("anotherpassword"), 1);
    ck_assert_msg(error == PPH_ERROR_OK,
        " this shouldn't have broken the test");
  }


  // 2) ask again, with the wrong password
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      "expected ACCOUNT_IS_INVALID");
  
  pph_destroy_context(context);
}
END_TEST





// This checks for a proper behavior when providing an existing username, 
// first, as the first and only username, then after having many on the list
START_TEST(test_check_login_proper_data) {
  
  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                          
  unsigned char password[] = "i'mnothere";
  unsigned char username[] = "nonexistentpassword";
  unsigned char anotheruser[] = "0anotheruser";
  unsigned int i;


  // setup the context 
  context = pph_init_context(threshold,isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  

  // add a single user and see how it behaves:
  // 1) add a user
  error = pph_create_account(context, username, strlen(username), password,
     strlen(password), 1);
  ck_assert_msg(error == PPH_ERROR_OK, " this shouldn't have broken the test");

  // 2) ask for it, providing correct credentials
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ERROR_OK, 
      "expected OK");
  
  
  // lets add a whole bunch of users and check for an existing one again
  // 1) add a whole new bunch of users:
  for(i=1;i<9;i++) {
    anotheruser[0] = i+48;
    error = pph_create_account(context, anotheruser, strlen(anotheruser),
        "anotherpassword", strlen("anotherpassword"), 1);
    ck_assert_msg(error == PPH_ERROR_OK,
        " this shouldn't have broken the test");
  }


  // 2) ask again, with the correct password
  error = pph_check_login(context, username, strlen(username), password,
      strlen(password));
  ck_assert_msg(error == PPH_ERROR_OK, 
      "expected ERROR_OK");
  
  pph_destroy_context(context);
}
END_TEST


// this test attempts to create full ranged usernames and passwords and check
// the login procedures at the same time.
START_TEST(test_pph_create_and_check_login_full_range) {
 
 
  pph_context *context;
  uint8 username_buffer[MAX_USERNAME_LENGTH];
  uint8 password_buffer[MAX_PASSWORD_LENGTH];
  unsigned int i;
  uint8 threshold = 2;
  uint8 isolated_check_bits = 0;
  PPH_ERROR error;


  context = pph_init_context( threshold, isolated_check_bits);
  ck_assert(context != NULL);

  // we will iterate all of the lengths of the username fields and generate 
  // random string users.
  for( i = 1; i < MAX_USERNAME_LENGTH; i++){
    
    // generate a username and a password of length i
    get_random_bytes(i, username_buffer);
    get_random_bytes(i, password_buffer);

    // create an account with those credentials
    error = pph_create_account( context, username_buffer, i, password_buffer, 
        i, 1);
    ck_assert( error == PPH_ERROR_OK );

    // check the login of the newly created account
    error = pph_check_login( context, username_buffer, i, password_buffer, i);
    ck_assert( error == PPH_ERROR_OK);

    // now invert the first byte of the password so we can't login
    password_buffer[0] = ~password_buffer[0];
    error = pph_check_login( context, username_buffer, i, password_buffer, i);
    ck_assert( error != PPH_ERROR_OK);

  }

  pph_destroy_context(context);

}END_TEST


////////// shamir recombination and persistent storage test cases. //////////

// this checks that the unlock password data correctly parses input.
START_TEST(test_pph_unlock_password_data_input_sanity) {
  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                          
  unsigned int i;
  unsigned int username_count=5;
  const uint8 *usernames[] = {"username1",
                              "username12",
                              "username1231",
                              "username26",
                              "username5",
                            };
  unsigned int username_lengths[] = { strlen("username1"),
                                      strlen("username12"),
                                      strlen("username1231"),
                                      strlen("username26"),
                                      strlen("username5"),
                                  };
  const uint8 *passwords[] = {"password1",
                              "password12",
                              "password1231",
                              "password26",
                              "password5"
                              };

  
  // check for bad pointers at first
  error = pph_unlock_password_data(NULL, username_count, usernames,
     username_lengths, passwords);
  ck_assert_msg(error == PPH_BAD_PTR," EXPECTED BAD_PTR");

  // setup the context 
  context = pph_init_context(threshold, isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  
  // let's imagine it's all broken
  context->is_bootstrapped = false;
  
  // now give a wrong username count, below the threshold.
  error = pph_unlock_password_data(context, 0, usernames, username_lengths,
      passwords);
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      " Expected ACCOUNT_IS_INVALID");

  // do it again, more graphical... 
  error = pph_unlock_password_data(context, threshold -1, usernames,
     username_lengths, passwords);
  ck_assert_msg(error == PPH_ACCOUNT_IS_INVALID, 
      " Expected ACCOUNT_IS_INVALID");

  // let's check for NULL pointers on the username and password fields
  error = pph_unlock_password_data(context, username_count, NULL, 
     username_lengths, passwords);
  ck_assert_msg(error == PPH_BAD_PTR," EXPECTED BAD_PTR");

 
  // check for wrong values in the username_lengths field
  error = pph_unlock_password_data(context, username_count, usernames, NULL,
      passwords);
  ck_assert( error == PPH_BAD_PTR);

  // let's check for NULL pointers on the username and password fields
  error = pph_unlock_password_data(context, username_count, usernames, 
      username_lengths, NULL);
  ck_assert_msg(error == PPH_BAD_PTR," EXPECTED BAD_PTR");

  pph_destroy_context(context);

}
END_TEST





// we check that the unlock password data cannot bootstrap the valut provided 
// wrong information. 
START_TEST(test_pph_unlock_password_data_correct_thresholds) {
  
  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
                          
  unsigned int i;
  unsigned int username_count=5;
  uint8 secret[DIGEST_LENGTH];

  const uint8 *usernames[] = {"username1",
                              "username12",
                              "username1231",
                              "username26",
                              "username5",
                            };
  const uint8 *passwords[] = {"password1",
                              "password12",
                              "password1231",
                              "password26",
                              "password5"
                              };

  unsigned int username_lengths[] = { strlen("username1"),
                                      strlen("username12"),
                                      strlen("username1231"),
                                      strlen("username26"),
                                      strlen("username5"),
                                  };
  const uint8 *usernames_subset[] = { "username12",
                                      "username26"};
  unsigned int username_lengths_subset[] = { strlen("username12"),
                                            strlen("username26"),
                                    };
  const uint8 *password_subset[] = {"password12",
                                    "password26"};
  const uint8 *bad_passwords[] = { "whoisthisguy?",
                                   "notauser"};
  
  
  
  // setup the context 
  context = pph_init_context(threshold,isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  
  //backup the secret
  memcpy(secret,context->secret,DIGEST_LENGTH);

  // create some usernames so we can bootstrap the context.
  for(i=0;i<username_count;i++) {
    pph_create_account(context,usernames[i], strlen(usernames[i]), passwords[i],
          strlen(passwords[i]),1);
  }


  // let's imagine it's all broken
  context->is_bootstrapped = false;
  strcpy(context->secret,"thiswasnotthesecretstring");

  // now give a correct full account information, we expect to have our secret
  // back. 
  error = pph_unlock_password_data(context, username_count, usernames,
      username_lengths, passwords);
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(secret[i] ==  context->secret[i]);
  }

  // let's imagine it's all broken (Again)
  context->is_bootstrapped = false;
  strcpy(context->secret,"thiswasnotthesecretstring");

  // now give a correct full account information, we expect to have our secret
  // back. 
  error = pph_unlock_password_data(context, 2, usernames_subset,
      username_lengths_subset, password_subset);
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(secret[i] ==  context->secret[i]);
  }


  // and last but not least, create a superuser account and bootstrap the
  // server by himself, note how he's got three shares assigned to his account.
  pph_create_account(context,"ipicklocks", strlen("ipicklocks"),"ipickpockets",
      strlen("ipickpockets"), 3);
  error = pph_unlock_password_data(context, 1 ,strdup("ipicklocks"),
      1, strdup("ipickpockets"));
  
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(secret[i] ==  context->secret[i]);
  }
  
  // attempt to bootstrap the store with wrong passwords
  free(context->secret);
  context->is_bootstrapped = false;

  error = pph_unlock_password_data(context, 2, usernames_subset,
     username_lengths_subset, bad_passwords);
  ck_assert(error == PPH_ACCOUNT_IS_INVALID);

  // clean up our mess
  pph_destroy_context(context);
}
END_TEST





// test input sanity on the store function
START_TEST(test_pph_store_context_input_sanity) {

  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
  unsigned int i;

  // wrong context pointer test
  error = pph_store_context(NULL,"pph.db");
  ck_assert_msg(error == PPH_BAD_PTR, " expected BAD_PTR");

  // initialize a context because we are going to give a valid pointer from now
  // on
  context = pph_init_context(threshold, isolated_check_bits);
  
  // wrong filename
  error = pph_store_context(context, NULL);
  ck_assert_msg(error == PPH_BAD_PTR, " expected BAD_PTR");

  // correct data
  error = pph_store_context(context, "pph.db");
  ck_assert_msg(error == PPH_ERROR_OK," expected ERROR_OK");

  pph_destroy_context(context);

}END_TEST





// test input sanity on the reload function
START_TEST(test_pph_reload_context_input_sanity) {

  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
  unsigned int i;


  // check for wrong filenames
  context = pph_reload_context(NULL);
  ck_assert_msg(context == NULL, " expected to break with a null argument");

  // check for nonexistent files.
  context = pph_reload_context("nonexistent_file");
  ck_assert_msg(context == NULL, " expected to break with a nonexistent file");

  // check for a valid file and consistent elements
  context = pph_reload_context("pph.db");
  ck_assert_msg(context != NULL, " this should have produced a valid pointer");
  ck_assert_msg(context->threshold == 2, " threshold didn't match the one set");
  ck_assert_msg(context->isolated_check_bits == 0, "isolated bytes don't match");
  ck_assert_msg(context->secret == NULL, " didn't store null for secret");
  ck_assert_msg(context->is_bootstrapped == false, " loaded a bootstrapped context");
  
  pph_destroy_context(context);

}END_TEST





// do a full lifecycle test, in other words, create a context with accounts, 
// store it, reload it, bootstrap to provide full login and creation service. 
START_TEST(test_pph_store_and_reload_with_users) {
  
  
  PPH_ERROR error;
  pph_context *context;
  uint8 threshold = 2; 
  uint8 isolated_check_bits = 0;
  uint8 secret[DIGEST_LENGTH]; 
  unsigned int i;
  unsigned int username_count=5;
  const uint8 *usernames[] = {"username1",
                              "username12",
                              "username1231",
                              "username26",
                              "username5",
                            };
  const uint8 *passwords[] = {"password1",
                              "password12",
                              "password1231",
                              "password26",
                              "password5"
                              };

  unsigned int username_lengths[] = { strlen("username1"),
                                      strlen("username12"),
                                      strlen("username1231"),
                                      strlen("username26"),
                                      strlen("username5"),
                                  };
  const uint8 *usernames_subset[] = { "username12",
                                      "username26"};
  unsigned int username_lengths_subset[] = { strlen("username12"),
                                            strlen("username26"),
                                            };

  const uint8 *password_subset[] = {"password12",
                                    "password26"};

  // setup the context 
  context = pph_init_context(threshold, isolated_check_bits);
  ck_assert_msg(context != NULL,
      "this was a good initialization, go tell someone");
  for(i=0;i<username_count;i++) {
    pph_create_account(context,usernames[i], strlen(usernames[i]),passwords[i],
          strlen(passwords[i]),1);
  }
 

  // backup our secret
  memcpy(secret,context->secret,DIGEST_LENGTH);

  // let's store our data!
  error = pph_store_context(context,"pph.db");
  ck_assert_msg(error == PPH_ERROR_OK, " couldn't store a context with users");
  

  // destroy the existing context in memory
  pph_destroy_context(context); 


  // reload the one in disk.
  context = pph_reload_context("pph.db");
  ck_assert_msg(context != NULL, " Didn't get a valid structure from disk");

  // now give a correct full account information, we expect to have our secret
  // back. 
  pph_account_node *user_nodes;
  user_nodes = context->account_data;
  error = pph_unlock_password_data(context, username_count, usernames,
      username_lengths, passwords);
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(secret[i]==context->secret[i]);
  }

  
  // clean up, we will reload the context. 
  pph_destroy_context(context);

  context = pph_reload_context("pph.db");
  ck_assert_msg(context != NULL, " didn't get a valid structure from disk");

  // now give a correct full account information, we expect to have our secret
  // back. 
  error = pph_unlock_password_data(context, 2, usernames_subset,
        username_lengths_subset, password_subset);
  for(i=0;i<DIGEST_LENGTH;i++) {
    ck_assert(secret[i]==context->secret[i]);
  }

  pph_destroy_context(context);
}
END_TEST





// we will check for a full input range bootstrap procedure using random
// username-password combinations of various lengths, the procedure should
// yield a correct secret and an incorrect secret when prompted with one bad
// password
START_TEST(test_pph_unlock_password_data_full_range) {
  
  
  uint8 *usernames[MAX_USERNAME_LENGTH];
  uint8 *passwords[MAX_PASSWORD_LENGTH];
  unsigned int *username_lengths;
  unsigned int i;
  PPH_ERROR error;
  uint8 threshold = 2;
  uint8 isolated_check_bits = 0;
  pph_context *context;

  // initialize the buffers
  username_lengths = malloc(sizeof(*username_lengths)*MAX_USERNAME_LENGTH);

  context = pph_init_context( threshold, isolated_check_bits);
  ck_assert(context != NULL);
  
  // initialize a username password pair of each length of a random value
  for( i = 0; i < MAX_USERNAME_LENGTH-1; i++){

    usernames[i] = malloc(sizeof(*usernames[i])*MAX_USERNAME_LENGTH);
    passwords[i] = malloc(sizeof(*passwords[i])*MAX_PASSWORD_LENGTH);
    ck_assert( usernames[i] != NULL);
    ck_assert( passwords[i] != NULL);

    get_random_bytes( i+1, usernames[i]);
    get_random_bytes( i+1, passwords[i]);
    
   
    username_lengths[i]= i + 1;
    
    error = pph_create_account( context, usernames[i], username_lengths[i],
        passwords[i], username_lengths[i], 1);
    ck_assert( error == PPH_ERROR_OK);
    
    error = pph_check_login( context, usernames[i], username_lengths[i],
        passwords[i], username_lengths[i]);
    ck_assert( error == PPH_ERROR_OK);

  }

  // lock the context
  context->is_bootstrapped = false;
  
  // unlock (bootstrap) the context
  error = pph_unlock_password_data( context, MAX_USERNAME_LENGTH -1, usernames,
      username_lengths, passwords);
  ck_assert( error == PPH_ERROR_OK );

  // check we can login after boostrapping 
  for(i = 0; i < MAX_USERNAME_LENGTH-1; i++){
  
    error = pph_check_login( context, usernames[i], username_lengths[i],
        passwords[i], username_lengths[i]);
    ck_assert(error == PPH_ERROR_OK);

  }

  // now, fail to bootstrap the context, 
  context->is_bootstrapped = false;
  passwords[0][0] = ~passwords[0][0];

  error = pph_unlock_password_data( context, MAX_USERNAME_LENGTH -1, usernames,
      username_lengths, passwords);
  ck_assert( error != PPH_ERROR_OK);

  // free everything
  for(i = 0; i < MAX_USERNAME_LENGTH-1; i++) {
    
    free(usernames[i]);
    free(passwords[i]);

  }

  free(username_lengths);
  pph_destroy_context(context);

}END_TEST;



// Define the suite.
Suite * polypasswordhasher_suite(void)
{
  Suite *s = suite_create ("buildup");

  /* no isolated verification case */
  TCase *tc_non_isolated = tcase_create ("non-isolated");

  // generating and checking secrets
  tcase_add_test (tc_non_isolated, test_generate_pph_secret_input_sanity);
  tcase_add_test (tc_non_isolated, test_check_pph_secret_input_sanity);
  tcase_add_test (tc_non_isolated, test_generate_and_check_pph_secret_mixed);
  
  // initializing contexts
  tcase_add_test (tc_non_isolated,test_pph_init_context_wrong_threshold);
  tcase_add_test (tc_non_isolated,test_pph_init_context_no_isolated_check_bits);
  
  // creating an account 
  tcase_add_test (tc_non_isolated,test_create_account_context);
  tcase_add_test (tc_non_isolated,test_create_account_usernames);
  tcase_add_test (tc_non_isolated,test_create_account_passwords);
  tcase_add_test (tc_non_isolated,test_create_account_sharenumbers);
  tcase_add_test (tc_non_isolated,test_create_account_entry_consistency);

  // checking for an existing account
  tcase_add_test (tc_non_isolated,test_check_login_input_sanity);
  tcase_add_test (tc_non_isolated,test_check_login_wrong_username);
  tcase_add_test (tc_non_isolated,test_check_login_wrong_password);
  tcase_add_test (tc_non_isolated,test_check_login_proper_data);
  tcase_add_test (tc_non_isolated,test_pph_create_and_check_login_full_range);
  suite_add_tcase (s, tc_non_isolated);

  /* bootsrapping (for both cases) */
  TCase *tc_unlock_shamir = tcase_create ("unlock_shamir");
  tcase_add_test (tc_unlock_shamir, test_pph_unlock_password_data_input_sanity);
  tcase_add_test (tc_unlock_shamir, 
        test_pph_unlock_password_data_correct_thresholds);
  tcase_add_test (tc_unlock_shamir, test_pph_unlock_password_data_full_range);
  suite_add_tcase (s, tc_unlock_shamir);

  /* pph context persistency tests */
  TCase *tc_store_context = tcase_create ("store_context");
  tcase_add_test( tc_store_context, test_pph_store_context_input_sanity);
  tcase_add_test( tc_store_context, test_pph_reload_context_input_sanity);
  tcase_add_test( tc_store_context, test_pph_store_and_reload_with_users);
  suite_add_tcase (s, tc_store_context);

  return s;

}




// setup the suite runner
int main (void)
{
  int number_failed;
  Suite *s =  polypasswordhasher_suite();
  SRunner *sr = srunner_create (s);
  srunner_run_all (sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed (sr);
  srunner_free (sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


