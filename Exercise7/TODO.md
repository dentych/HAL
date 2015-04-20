Init skal:
* Alloc mem region, som returnerer devno.
* class_create.
* Sætte class attr.
* device_create til vores led.

Store og show metoder:
* Store kaldes når der skrives til devicet.
* Show kaldes når der læses fra devicet.

Exit metode:
* Deallokere memory
* device_destroy
* class_destroy
