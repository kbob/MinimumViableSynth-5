CoreMIDI object hierarchy

Inheritance hierarchy is flat.

    object
        client
        port, owned by client
        device
        entity, owned by device
        endpoint, owned by entity

Structure is not flat.

    client
        port

    device
        entity
            endpoint

    ... and then via properties endpoints connect to further devices.
