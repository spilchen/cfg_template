#include <map>
#include <memory>
#include <algorithm>


/// Helper to convert a string value to a bool
bool strToBool(const std::string in)
{
    std::string upStr = in;
    std::transform(upStr.begin(), upStr.end(), upStr.begin(), ::toupper);
    if (upStr == "0" || upStr == "FALSE" || upStr == "OFF") {
        return false;
    } else {
        return true;
    }
}

/**
   Abstract config value.

   Defines all of the APIs for read-only and writeable config values.
   Specific subclass will add a specific type and implement the APIs.
*/
class AbstractCV {
    /// String representation of the config value.  Used for debug purposes
    std::string mKey;
    /// Help text to describe the type of values can be set for the config
    /// value and how the config value is used.
    std::string mHelp;

public:
    /// Constructor
    /// @param[in] key String name of the config value
    /// @param[in] help Help text to describe the config value
    AbstractCV(const std::string& key, const std::string& help)
        : mKey(key), mHelp(help)
    {
    }

    /// Return the help text
    virtual const std::string& help() const { return mHelp; }

    /// Return the string name of the config parm
    virtual const std::string& key() const { return mKey; }

    /// Return the value as a string
    virtual std::string asStr() const = 0;

    /// Return the value as 64-bit int
    virtual int64_t asInt() const = 0;

    /// Return the value as a bool
    virtual bool asBool() const = 0;

    /// Set a new config value.  
    /// Default value is to prevent the set.  This can be overridden in a subclass.
    virtual void set(const std::string& v) { 
        throw std::runtime_error("Read-only config value.  Set is not supported: " + key());
    }
};

/**
   Read-only config value that holds an arbitrary integer type

   set() will throw if called.
*/
template <typename IntType>
class IntReadOnlyCV : public AbstractCV {
    IntType mVal;

public:
    IntReadOnlyCV(IntType defVal, const std::string& key, const std::string& help) 
        : AbstractCV(key, help), mVal(defVal)
    {
    }

    virtual std::string asStr() const override { return std::to_string(mVal); }
    virtual int64_t asInt() const override { return mVal; }
    virtual bool asBool() const override { return (mVal) ? true : false; }
};

/**
   Updatable config value that holds an arbitrary integer type.

   The integer type is wrapped in an atomic to allow for concurrent updates.
*/
template <typename IntType>
class IntUpdatableCV : public AbstractCV {
    std::atomic<IntType> mVal;

public:
    IntUpdatableCV(IntType defVal, const std::string& key, const std::string& help) 
        : AbstractCV(key, help), mVal(defVal)
    {
    }

    virtual std::string asStr() const override { return std::to_string(mVal.load()); }
    virtual int64_t asInt() const override { return mVal.load(); }
    virtual bool asBool() const override { return (mVal.load()) ? true : false; }
    virtual void set(const std::string& v) override {
        mVal = std::stoi(v);
    }
};

/**
   Read-only boolean config value

   Set throws an exception if called.
*/
class BoolReadOnlyCV : public AbstractCV {
    bool mVal;

public:
    BoolReadOnlyCV(bool defVal, const std::string& key, const std::string& help) 
        : AbstractCV(key, help), mVal(defVal)
    {
    }

    virtual std::string asStr() const override { return mVal ? "true" : "false"; }
    virtual int64_t asInt() const override { return mVal; }
    virtual bool asBool() const override { return mVal; }
};

/**
   Read-only string config value.

   Set throws an exception if called.
*/
class StrReadOnlyCV : public AbstractCV {
    std::string mVal;

public:
    StrReadOnlyCV(const std::string& defVal, const std::string& key, const std::string& help) 
        : AbstractCV(key, help), mVal(defVal)
    {
    }

    virtual std::string asStr() const override { return mVal; }
    virtual int64_t asInt() const override { return std::stoi(mVal); }
    virtual bool asBool() const override { return strToBool(mVal); }
};

/**
   Factory class that generates config values

   Handles picking of the initial value by using the hard-coded default and the
   value defined in a map.
*/
class CVFactory
{
    /// Key/value pairs of override values.  The key names are the config value
    /// parameter names.  If a value is missing from this map, then we will
    /// just use the hard coded default value.
    const std::map<std::string, std::string>& mOverrides;

public:
    /// Constructor
    /// @param[in] override Pairs of key/value that override specific config values.
    CVFactory(const std::map<std::string, std::string>& overrides)
        : mOverrides(overrides)
    {
    }

    /// Make a read-only config value internally stored as an integer
    template <typename IntType>
    std::shared_ptr<AbstractCV>
    Make_IntReadOnlyCV(const std::string& key, IntType defVal, const std::string& help)
    {
        std::shared_ptr<AbstractCV> p(new IntReadOnlyCV<IntType>(resolveVal(key, defVal), key, help));
        return p;
    }

    /// Make a read-only config value internally stored as a string
    std::shared_ptr<AbstractCV>
    Make_StrReadOnlyCV(const std::string& key, const std::string& defVal, const std::string& help)
    {
        std::shared_ptr<AbstractCV> p(new StrReadOnlyCV(resolveVal(key, defVal), key, help));
        return p;
    }

    /// Make a updatable config value internally stored as an integer
    template <typename IntType>
    std::shared_ptr<AbstractCV>
    Make_IntUpdatableCV(const std::string& key, IntType defVal, const std::string& help)
    {
        std::shared_ptr<AbstractCV> p(new IntUpdatableCV<IntType>(resolveVal(key, defVal), key, help));
        return p;
    }

    /// Make a read-only config value internally stored as a bool
    std::shared_ptr<AbstractCV>
    Make_BoolReadOnlyCV(const std::string& key, const bool defVal, const std::string& help)
    {
        std::shared_ptr<AbstractCV> p(new BoolReadOnlyCV(resolveVal(key, defVal), key, help));
        return p;
    }

private:
    /// Resolve the initial value for an integer config value
    /// If override value exists, we'll use that otherwise use the default value passed in
    template <typename IntType>
    const IntType resolveVal(const std::string& overrideKey, const IntType defValue)
    {
        auto it = mOverrides.find(overrideKey);
        if (it == mOverrides.end()) {
            return defValue;
        } else {
            return std::stoi(it->second);
        }
    }

    /// Resolve the initial value for an string config value
    /// If override value exists, we'll use that otherwise use the default value passed in
    const std::string& resolveVal(const std::string& overrideKey, const std::string& defValue)
    {
        auto it = mOverrides.find(overrideKey);
        if (it == mOverrides.end()) {
            return defValue;
        } else {
            return it->second;
        }
    }

    /// Resolve the initial value for bool config value
    /// If override value exists, we'll use that otherwise use the default value passed in
    bool resolveVal(const std::string& overrideKey, const bool defValue)
    {
        auto it = mOverrides.find(overrideKey);
        if (it == mOverrides.end()) {
            return defValue;
        } else {
            return strToBool(it->second);
        }

    }
};

/**
   The template class to hold a set of config parameters.

   Each config parameter has an associated config value that you use to get/set
   its value.
*/
template <typename TConfigEnum>
class ConfigTemplate
{
    /// Factory object that is used to create the AbstractCV subclass
    CVFactory mFactory;
    /// Map of config params and the AbstractCV to get/set their values
    mutable std::map<TConfigEnum, std::shared_ptr<AbstractCV>> mParms;

public:
    /// Constructor
    /// Each instantiation of the template will use template specialization to
    /// seed the mParms map.
    ///
    /// @param[in] overrides List of key/value pairs for specific config parms
    ///            that are being overridden.  For each pair, it will override
    ///            the hard-coded default.
    ConfigTemplate(std::map<std::string, std::string> overrides);

    /// Get a config value as a specific type
    /// @tparam T The type to get the value as.
    /// @param[in] parm Config parm to lookup
    template <typename T>
    T as_(TConfigEnum parm) const {
        auto& val = *(mParms[parm]);
        T returnVal;
        convertToType(val, returnVal);
        return returnVal;
    }

    /// Set a config value
    /// This will throw an exception if this used with a read-only config value
    /// @param[in] parm Config parm to set
    /// @param[in] newVal New value to set.
    void set(TConfigEnum parm, const std::string& newVal) {
        auto& val = *(mParms[parm]);
        val.set(newVal);
    }

private:
    /// Convert a config value to a string type
    void convertToType(const AbstractCV& val, std::string& returnVal) const {
        returnVal = val.asStr();
    }

    /// Convert a config value to an integer type
    template <typename IntType>
    void convertToType(const AbstractCV& val, IntType& returnVal) const {
        returnVal = static_cast<IntType>(val.asInt());
    }

    /// Convert a config value to a boolean type
    void convertToType(const AbstractCV& val, bool& returnVal) const {
        returnVal = val.asBool();
    }
};
