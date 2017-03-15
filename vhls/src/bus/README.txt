Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>



busmux64   A model of a generic bus that can either actually queue bus transactions to model
           contention or else add delays according to the measured current local utlisation with respect
           to the maximum throughput of the bus.

hypertransport   A model of AMD's hypertransport link that can be used to interconnect system components.

SRI        A model of AMD's system request interface that funnels the requests from several CPU nodes to
           a network-on-chip component, such as crossbar.

crossbar   A model of a system-on-chip switch element that can be wired into various standard network
           topologies to model contention arising from asymmetric traffice flo.

vmunit     A model of a component that maps virtual to physical addresses.
 
