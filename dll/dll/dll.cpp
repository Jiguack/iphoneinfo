// dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "dll.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/diagnostics_relay.h>

static void print_xml(plist_t node)
{
	char *xml = NULL;
	uint32_t len = 0;
	plist_to_xml(node, &xml, &len);
	if (xml) {
		puts(xml);
	}
}

int infoString(char** result, const char* domain, const char* key, const char *udid) {
	lockdownd_client_t client = NULL;
	lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
	idevice_t device = NULL;
	idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
	int i;
	int simple = 0;
	char *xml_doc = NULL;
	uint32_t xml_length;
	plist_t node = NULL;

	ret = idevice_new(&device, udid);
	if (ret != IDEVICE_E_SUCCESS) {
		return -1;
	}

	if (LOCKDOWN_E_SUCCESS != (ldret = simple ?
		lockdownd_client_new(device, &client, "ideviceinfo") :
		lockdownd_client_new_with_handshake(device, &client, "ideviceinfo"))) {
		//fprintf(stderr, "ERROR: Could not connect to lockdownd, error code %d\n", ldret);
		idevice_free(device);
		return -1;
	}

	/* run query and output information */
	if (lockdownd_get_value(client, domain, key, &node) == LOCKDOWN_E_SUCCESS) {
		if (node) {
			plist_get_string_val(node, result);

			plist_free(node);
			node = NULL;
		}
	}

	lockdownd_client_free(client);
	idevice_free(device);

	return 0;
}

int infoInt(const char* domain, const char* key, const char *udid) {
	lockdownd_client_t client = NULL;
	lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
	idevice_t device = NULL;
	idevice_error_t ret = IDEVICE_E_UNKNOWN_ERROR;
	int i;
	int simple = 0;
	char *xml_doc = NULL;
	uint32_t xml_length;
	plist_t node = NULL;
	int result = 0;

	ret = idevice_new(&device, udid);
	if (ret != IDEVICE_E_SUCCESS) {
		return -3;
	}

	if (LOCKDOWN_E_SUCCESS != (ldret = simple ?
		lockdownd_client_new(device, &client, "ideviceinfo") :
		lockdownd_client_new_with_handshake(device, &client, "ideviceinfo"))) {
		//fprintf(stderr, "ERROR: Could not connect to lockdownd, error code %d\n", ldret);
		idevice_free(device);
		return -1;
	}

	/* run query and output information */
	if (lockdownd_get_value(client, domain, key, &node) == LOCKDOWN_E_SUCCESS) {
		if (node) {
			uint64_t val;
			plist_t  val_node = NULL;
			if (key != NULL) {
				plist_get_uint_val(node, &val);
			}
			else {
				val_node = plist_dict_get_item(node, key);
				plist_get_uint_val(val_node, &val);
			}
			result = val;
			//result = EXIT_SUCCESS;
			plist_free(node);
			node = NULL;
		}
	}
	lockdownd_client_free(client);
	idevice_free(device);

	return result;
}

//get integer value by key from idevicediagnostics
int diagInt(const char* type, const char* key, const char *udid) {
	idevice_t device = NULL;
	lockdownd_client_t lockdown_client = NULL;
	diagnostics_relay_client_t diagnostics_client = NULL;
	plist_t node = NULL;
	lockdownd_error_t ret = LOCKDOWN_E_UNKNOWN_ERROR;
	lockdownd_service_descriptor_t service = NULL;
	int result = 0;

	if (IDEVICE_E_SUCCESS != idevice_new(&device, udid)) {
		return -3;
	}

	if (LOCKDOWN_E_SUCCESS != (ret = lockdownd_client_new_with_handshake(device, &lockdown_client, "idevicediagnostics"))) {
		idevice_free(device);
		printf("ERROR: Could not connect to lockdownd, error code %d\n", ret);
		goto cleanup;
	}

	/*  attempt to use newer diagnostics service available on iOS 5 and later */
	ret = lockdownd_start_service(lockdown_client, "com.apple.mobile.diagnostics_relay", &service);
	if (ret != LOCKDOWN_E_SUCCESS) {
		/*  attempt to use older diagnostics service */
		ret = lockdownd_start_service(lockdown_client, "com.apple.iosdiagnostics.relay", &service);
	}

	lockdownd_client_free(lockdown_client);

	if ((ret == LOCKDOWN_E_SUCCESS) && service && (service->port > 0)) {
		if (diagnostics_relay_client_new(device, service, &diagnostics_client) != DIAGNOSTICS_RELAY_E_SUCCESS) {
			printf("Could not connect to diagnostics_relay!\n");
			result = -1;
		}
		else {

			if (diagnostics_relay_request_diagnostics(diagnostics_client, type, &node) == DIAGNOSTICS_RELAY_E_SUCCESS) {
				if (node) {
					uint64_t val;
					plist_t  val_node;
					val_node = plist_dict_get_item(node, type);
					//print_xml(val_node);
					val_node = plist_dict_get_item(val_node, key);
					plist_get_uint_val(val_node, &val);
					result = val;
					//result = EXIT_SUCCESS;
				}
			}
			else {
				printf("Unable to retrieve diagnostics from device.\n");
				return -2;
			}

			diagnostics_relay_goodbye(diagnostics_client);
			diagnostics_relay_client_free(diagnostics_client);
		}
	}

cleanup:
	if (node) {
		plist_free(node);
	}
	return result;
}

int icount(void) {
	char **dev_list = NULL;
	int i;
	int count = 0;

	if (idevice_get_device_list(&dev_list, &i) < 0) {
		fprintf(stderr, "ERROR: Unable to retrieve device list!\n");
		return -1;
	}
	for (i = 0; dev_list[i] != NULL; i++) {
		count++;
		//printf("%s\n", dev_list[i]);
	}
	idevice_device_list_free(dev_list);
	return count;
}

int ilist(void) {
	char **dev_list = NULL;
	int i;
	int count = 0;

	if (idevice_get_device_list(&dev_list, &i) < 0) {
		fprintf(stderr, "ERROR: Unable to retrieve device list!\n");
		return -1;
	}
	printf("[");
	for (i = 0; dev_list[i] != NULL; i++) {
		count++;
		if (count>1)
			printf(",");
		printf("\'%s\'", dev_list[i]);
	}
	printf("]");
	idevice_device_list_free(dev_list);
	return count;
}

void iinfo(void) {
	iinfo_id(NULL);
}

void iinfo_id(const char* id) {
	char *product_type = NULL;
	char *product_ver = NULL;

	infoString(&product_type, NULL, "ProductType", id);
	infoString(&product_ver, NULL, "ProductVersion", id);

	if (product_type != NULL && product_ver != NULL) {
		printf("{\'ProductType\':\'%s\'", product_type);
		printf(",");
		printf("\'ProductVersion\':\'%s\'}", product_ver);
	}
	else {
		printf("{\'info\':\'fail\'}");
	}
}

void ibattery(void) {
	ibattery_id(NULL);
}

void ibattery_id(const char* id) {
	char **dev_list = NULL;
	int i;
	int data_d = 0;
	int data_f = 0;
	int data_c = 0;
	data_d = diagInt("GasGauge", "DesignCapacity", id);
	data_f = diagInt("GasGauge", "FullChargeCapacity", id);
	data_c = infoInt("com.apple.mobile.battery", "BatteryCurrentCapacity", id);
	if (data_d < 0 || data_f < 0) {
		printf("{\'id\':\'fail\'}");
		return;
	}

	printf("{");
	if (id != NULL) {
			printf("\'id\':\'%s\'", id);
			printf(",");
	}
	else if (idevice_get_device_list(&dev_list, &i) < 0 && i == 1) {
		printf("\'id\':\'%s\'", dev_list[0]);
		printf(",");
	}
	printf("\'DesignCapacity\':\%d", data_d);
	printf(",");
	printf("\'FullChargeCapacity\':%d", data_f);
	printf(",");
	printf("\'BatteryCurrentCapacity\':%d", data_c);
	printf("}");
}








