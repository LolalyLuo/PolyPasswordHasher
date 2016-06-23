#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>

const struct pam_conv conv = {
	misc_conv,
	NULL
};

int main(int argc, char *argv[]) {
	pam_handle_t* pamh = NULL;
	int retval;
	const char* user = "nobody";
	printf("1=================================================\n");
	if(argc != 2) {
		printf("Usage: app [username]\n");
		exit(1);
	}

	user = argv[1];
	printf("2=================================================\n");
	retval = pam_start("check_user", user, &conv, &pamh);
	printf("3=================================================\n");
     	pam_set_item(pamh, PAM_AUTHTOK, "lolaly!!!");	
	printf("4=================================================\n");
	// Are the credentials correct?
	if (retval == PAM_SUCCESS) {
		printf("Credentials accepted.\n");
		retval = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK);
	}
	printf("5=================================================\n");
	// Can the accound be used at this time?
	if (retval == PAM_SUCCESS) {
		printf("Account is valid.\n");
		retval = pam_acct_mgmt(pamh, 0);
	}
	printf("6=================================================\n");
	// Did everything work?
	if (retval == PAM_SUCCESS) {
		printf("Authenticated\n");
	} else {
		printf("Not Authenticated\n");
	}	

	printf("7=================================================\n");
	//change password
	if (retval == PAM_SUCCESS) {
		printf("Now you have to change your password\n");
		do {
            		retval = pam_chauthtok(pamh, 0);
			printf("error message: %s\n", pam_strerror(pamh, retval));
        	} while (retval == PAM_AUTHTOK_ERR);
		
	}
	
	// close PAM (end session)
	if (pam_end(pamh, retval) != PAM_SUCCESS) {
		pamh = NULL;
		printf("check_user: ailed to release authenticator\n");
		exit(1);
	}
	printf("8=================================================\n");
	return retval == PAM_SUCCESS ? 0 : 1;
}
