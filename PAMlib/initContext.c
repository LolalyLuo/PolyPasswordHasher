#include <libpolypasswordhasher.h>

int main() {
	pph_context *context;
  	printf("initial context\n");
  	uint8 threshold;
  	printf("Enter the number of threshold (from 1 to MAX_ACCOUNTS),recommand 2\n");
  	scanf("%d",&threshold);   
                          
  	uint8 isolated_check_bits;
  	printf("Enter the number of threshold (from 1 to MAX_ACCOUNTS),recommand 2\n");
  	scanf("%d",&isolated_check_bits);

	context = pph_init_context(threshold, isolated_check_bits);
	
	pph_create_account(context, "Lolaly", strlen("Lolaly"), "Lolaly123", strlen("Lolaly123"),1);
	pph_create_account(context, "Santiago", strlen("Santiago"), "Santiago123", strlen("Santiago123"),1);
	pph_store_context(context, "PPHdata");
	pph_destroy_context(context);
	printf("context is initialed and stored!\n");
}
