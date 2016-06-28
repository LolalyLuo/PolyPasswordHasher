#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PAM_SM_PASSWORD
#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#define PAM_SM_SESSION
#include <syslog.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>



/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	pam_syslog(pamh, LOG_NOTICE, "PPH: Set credenticial successed!\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	pam_syslog(pamh, LOG_NOTICE, "PPH: Account management successed!\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	pam_syslog(pamh, LOG_NOTICE, "PPH: Open session successed!\n");
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	pam_syslog(pamh, LOG_NOTICE, "PPH: Close session successed!\n");
	return PAM_SUCCESS;
}

//authentication 
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
	struct pam_conv *conv;
	struct pam_message msg;
	const struct pam_message *msgp;
	struct pam_response *resp;

	int retval, pam_err;
	const char* username;
	const char* password;
	//checking username and setup syslog
	retval = pam_get_user(pamh, &username, "Username(pam): ");	
	openlog("PAM_PPH: ",LOG_NOWAIT, LOG_LOCAL1);
	if (retval != PAM_SUCCESS) {
		pam_syslog(pamh, LOG_ERR, "Error: can't access username. Lolaly \n");
		return PAM_SUCCESS;
	}
	//get password 
	pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, &password, "What is your password: ");
	if (pam_err != PAM_SUCCESS){
		pam_syslog(pamh, LOG_ERR, "Error: can't get password. Lolaly \n");
		return PAM_SUCCESS;
	}
	
	return PAM_SUCCESS;
}



//change password
PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv){
	
	return PAM_SUCCESS;
}




