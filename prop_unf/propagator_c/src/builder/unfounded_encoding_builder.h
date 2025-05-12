#include "encoding_builder.h"

class UnfoundedEncodingBuilder: public EncodingBuilder{

public:
    std::string buildProgram(NonHFCPropagator* prop) override;
    EncodingBuilder* withWeightRule(NonHFCPropagator* prop, const WeightRule* rule) override;
    EncodingBuilder* withRule(NonHFCPropagator* prop,Rule* rule) override;

    UnfoundedEncodingBuilder* withHandC(NonHFCPropagator* prop) ;
};