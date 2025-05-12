#include "reduct_encoding_builder.h"
// new
class AspReductEncodingBuilder: public ReductEncodingBuilder{


    public:
        virtual std::string buildProgram(NonHFCPropagator* prop) override;
        virtual EncodingBuilder* withWeightRule(NonHFCPropagator* prop, const WeightRule* rule) override;
        virtual EncodingBuilder* withRule(NonHFCPropagator* prop,Rule* rule) override; 
    
};

