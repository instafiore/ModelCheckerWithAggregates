#pragma once
#include "encoding_builder.h"

class ReductEncodingBuilder: public EncodingBuilder{

public:
    virtual std::string buildProgram(NonHFCPropagator* prop) override;
    virtual EncodingBuilder* withWeightRule(NonHFCPropagator* prop, const WeightRule* rule) override;
    virtual EncodingBuilder* withRule(NonHFCPropagator* prop,Rule* rule) override;

    ReductEncodingBuilder* withConstraintAssumptions(NonHFCPropagator* prop) ;
};


