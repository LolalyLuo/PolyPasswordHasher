#include <stdio.h>
#define PAM_SM_PASSWORD
#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>


const char *password;
const char *right_password;

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	printf("Account Management\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	printf("Session is open\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	printf("Session is closed\n");
	return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
	struct pam_conv *conv;
	struct pam_message msg;
	const struct pam_message *msgp;
	struct pam_response *resp;

	int retval, pam_err;
	const char* pUsername;
	
	//checking username and setup syslog
	retval = pam_get_user(pamh, &pUsername, "Username: ");
	printf("Welcome %s\n", pUsername);
		
	openlog("pam_pph(login/out): ", LOG_NOWAIT, LOG_LOCAL1);

	if (retval != PAM_SUCCESS) {
		printf("Error: can't access username.\n");
		syslog(LOG_INFO, "Error: can't access username. Lolaly \n");
		return retval;
	}

	//get password 
	pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, &password, "Give me a password: ");
	if (right_password == NULL){
		right_password = password;}
	if (pam_err != PAM_SUCCESS){
		printf("Error: can't get password.\n");
		syslog(LOG_INFO, "Error: can't get password. Lolaly \n");
		return PAM_SUCCESS;
	}
	printf("Got password: %s\n" , password); 
	if (strcmp(password, right_password) != 0){
		printf("You got the WRONG password!!!\n");
		syslog(LOG_INFO, "Can't access account\n");
		return PAM_AUTH_ERR;
	}
	printf("you got the RIGHT password!!!\n");
	syslog(LOG_INFO, "Open session by Lolaly \n");
	return PAM_SUCCESS;	
}
//change password
PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv){
	printf("get called!!\n");
	int pam_err;
	pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, &right_password, "Give me a NEW password: ");
	if (pam_err != PAM_SUCCESS){
		printf("Error: can't get password.\n");
		syslog(LOG_INFO, "Error: can't get password. Lolaly \n");
		return PAM_SUCCESS;
	}
	printf("Got password: %s\n" , right_password); 
	printf("call is over!!!\n");
	return PAM_SUCCESS;
}
