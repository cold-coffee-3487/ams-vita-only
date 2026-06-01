# AMS VITA 49.2 Tailoring Notes

## Legend / Terminology

The `Bit Used?` column specifies whether a bit, and where relevant the related field, is used within the base set.

- **B (Base Set):** Member of the baseline set. These bit- or sub-fields should be set to the correct value when a packet is assembled. In the body of Control, Ack, and Context packets, CIF bits with `B` indicate that the CIF bit shall be set to 1 and the associated variable field shall be present.

- **E (Extension Set):** Member of the extended set allowed by standard, but not required in a baseline implementation. In CIFs, bits with `E` should be set to 0 by default, and their related variable fields excluded from base set packets unless explicitly supported.

- **N (Not Allowed):** Not allowed. The bit or field is not in the base or extension set and is not used. In CIFs, bits with `N` should be set to 0, and related fields excluded.

## Field Sizes

`Field Size (words)` only applies to variable payload fields dynamically appended to the packet via Control/Context Indicator Fields (CIFs). It defines the number of 32-bit words the field occupies.

Fixed structural packet headers do not have a `Field Size (words)` property. Their boundaries and sizes must be derived directly from the explicit `Word` and `Bit` / `Bit Used` bounds defined in their respective packet tables.
