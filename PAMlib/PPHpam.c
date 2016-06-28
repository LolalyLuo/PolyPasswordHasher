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



void change_shadowfile(const char *user, const char *password);
/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){

	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	return PAM_SUCCESS;
}

/* expected hook, this is where custom stuff happens */
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
		return retval;
	}
	//get password 
	pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, &password, "What is your password: ");
	if (pam_err != PAM_SUCCESS){
		pam_syslog(pamh, LOG_ERR, "Error: can't get password. Lolaly \n");
		return pam_err;
	}
	return PAM_SUCCESS;

/*	//find the user
	FILE *shadow;
	shadow = fopen("/etc/shadow", "r");
	char buffer[4096];
	while(fgets(buffer, sizeof buffer, shadow) != NULL)  {
		if(strncmp(buffer, username, strlen(username)) == 0) {
			printf("*****Buffer: %s\n", buffer);
			const char s[2] = ":";
			char *theUser = strtok(buffer, s);
			char *sigRightPwd = strtok(NULL, s);
			char sig[5], rightPwd[1024];
			strncpy(sig, sigRightPwd, 5);
			printf("*****signature: %s\n", sig);
			if (strcmp("$PPH$", sig) != 0){
				printf("*****return\n");
				return PAM_CRED_INSUFFICIENT;
			}
			strcpy(rightPwd, sigRightPwd+5);
			printf("*****password: %s\n", rightPwd);
			printf("*****input pasword:%s\n", password);
			if(strcmp(rightPwd, password)== 0 ){
				printf("*****success\n");
				return PAM_SUCCESS;
			}
			else {
				return PAM_AUTH_ERR;
			}
		}
	}
	return PAM_USER_UNKNOWN;
*/
}

//change password
PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv){
	printf("get called!!\n");
	int pam_err;
	const char *user;
	const char *password;
	if (pam_get_user(pamh, &user, NULL) == PAM_SUCCESS){
		printf("Getting token for: %s\n", user);
	}
	pam_err = pam_get_authtok(pamh, PAM_AUTHTOK, &password, "Give me a NEW password: ");
	if (pam_err != PAM_SUCCESS){
		printf("Error: can't get password.\n");
		syslog(LOG_INFO, "Error: can't get password. Lolaly \n");
		return PAM_SUCCESS;
	}
	printf("Got password: %s\n" , password); 
	change_shadowfile(user, password);
	printf("call is over!!!\n");
	return PAM_SUCCESS;
}


void change_shadowfile(const char *user, const char *password) {	
	FILE *shadow, *temp;
	shadow = fopen("/etc/shadow", "rt+");
	temp = fopen("/etc/temp", "w+");
	char buffer[4096];
		
	if (shadow == NULL){
		printf("Can't open the file!!\n");
		exit(1);
	}
	printf("Changing shadowfile...\n");
	
	while(fgets(buffer, sizeof buffer, shadow) != NULL)  {
		if(strncmp(buffer, user, strlen(user)) == 0) {
			const char s[2] = ":";
			char *theUser = strtok(buffer, s);
			char *oldPwd = strtok(NULL, s );
			char *rest = strtok(NULL, "\0" );
			fprintf (temp, "%s:$PPH$%s:%s",theUser, password, rest);
		}
		else {
			fputs(buffer, temp);
		}
	}
	remove("/etc/shadow");
	rename("/etc/temp", "/etc/shadow");
	fclose(shadow);
	fclose(temp);
}



